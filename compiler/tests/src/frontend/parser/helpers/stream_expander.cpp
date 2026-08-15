#include "stream_expander.h"
#include <algorithm>
#include <variant>
#include <gtest/gtest.h>
#include "recovery_sentinel.h"
#include "deterministic_sampler.h"
#include "modifier_context_augmentation_manager.h"
#include "../expansion_and_sentinels/context_tree_walker.h"

namespace valuascript::compiler::test
{
    namespace
    {
        struct ResolvedBranchState
        {
            const ContextOverrideAny* match = nullptr;
            bool skip = false;
            UniversalVerifier inner_verifier;
            std::optional<std::vector<ParserExpectedError>> branch_errors;
            std::vector<SentinelKind> combined_excluded;
            std::vector<SentinelKind> combined_accepted;
        };

        ResolvedBranchState resolve_branch_state(const ProcessingItem& item, const Context& ctx)
        {
            ResolvedBranchState state;
            for (const auto& ov : item.context_overrides)
            {
                if (ov.context_name == ctx.name)
                {
                    state.match = &ov;
                    break;
                }
            }

            bool skip_by_override = (item.depth > 0) && state.match && state.match->skip_after_depth_0;
            state.skip = item.is_skipped || skip_by_override ||
                         std::find(item.skip_contexts.begin(), item.skip_contexts.end(), ctx.name) != item.skip_contexts.end();

            const ContextOverrideAny* verifier_match = (item.depth == 0) ? state.match : nullptr;

            state.inner_verifier = (verifier_match && verifier_match->verifier.has_value())
                                       ? verifier_match->verifier.value()
                                       : item.verifier;

            state.branch_errors = (verifier_match && verifier_match->errors.has_value())
                                      ? verifier_match->errors
                                      : item.custom_errors;

            state.combined_excluded = item.excluded_sentinels;
            if (state.match && !state.match->excluded_sentinels.empty())
            {
                for (auto k : state.match->excluded_sentinels)
                {
                    if (std::find(state.combined_excluded.begin(), state.combined_excluded.end(), k) == state.combined_excluded.end())
                    {
                        state.combined_excluded.push_back(k);
                    }
                }
            }

            state.combined_accepted = item.accepted_sentinels;
            if (state.match && !state.match->accepted_sentinels.empty())
            {
                state.combined_accepted = state.match->accepted_sentinels;
            }

            if (!state.combined_accepted.empty() && !state.combined_excluded.empty())
            {
                std::vector<SentinelKind> filtered;
                for (auto k : state.combined_accepted)
                {
                    if (std::find(state.combined_excluded.begin(), state.combined_excluded.end(), k) == state.combined_excluded.end())
                    {
                        filtered.push_back(k);
                    }
                }
                state.combined_accepted = std::move(filtered);
            }

            return state;
        }
    }

    std::vector<ProcessingItem> StreamExpander::apply_context_augmentations(
        InjectableType type,
        const std::string& snippet,
        const UniversalVerifier& verifier,
        const std::string& group_name,
        const std::vector<std::string_view>& skip_contexts,
        const std::vector<ContextOverrideAny>& context_overrides,
        const std::vector<SentinelKind>& excluded_sentinels,
        const std::vector<SentinelKind>& accepted_sentinels
    )
    {
        ProcessingItem base_item{
            .type = type, .code = snippet, .verifier = verifier, .path_name = group_name, .cumulative_prefix = "",
            .depth = 0, .recursion_depth = 0, .skip_contexts = skip_contexts, .is_skipped = false,
            .context_overrides = context_overrides, .custom_errors = std::nullopt,
            .excluded_sentinels = excluded_sentinels, .accepted_sentinels = accepted_sentinels
        };

        switch (type)
        {
        case InjectableType::Modifier:
            return ModifierContextAugmentationManager::generate_variations(base_item);

        case InjectableType::Import:
        case InjectableType::Directive:
        case InjectableType::Function:
        case InjectableType::Struct:
        case InjectableType::Enum:
        case InjectableType::TypeAlias:
        case InjectableType::Expression:
        case InjectableType::TypeAnnotation:
        case InjectableType::WeakStatement:
        case InjectableType::StrongStatement:
        case InjectableType::TopLevel:
        default:
            return {base_item};
        }
    }

    void StreamExpander::expand_to_top_level_stream(std::vector<ProcessingItem> items,
                                                    const ExpansionCallback& callback,
                                                    bool inject_sentinels,
                                                    std::optional<ExpansionPolicy> policy_override)
    {
        for (auto& var_item : items)
        {
            ContextTreeWalker<ProcessingItem>::Callbacks cb;
            cb.get_type = [](const ProcessingItem& item) { return item.type; };
            cb.on_terminal = [&](ProcessingItem item)
            {
                if (!item.is_skipped)
                {
                    callback(std::move(item));
                }
            };
            cb.on_normal_branch = [](const ProcessingItem& item, const Context& ctx, int next_rec_depth)
                -> std::vector<ProcessingItem>
                {
                    auto branch = resolve_branch_state(item, ctx);

                    UniversalVerifier final_verifier = branch.inner_verifier;
                    bool needs_transform = true;

                    bool is_injectable_inner_verifier = is_injectable_payload_for_context(
                        branch.inner_verifier, ctx.input_types);

                    if (branch.match && branch.match->verifier.has_value() && (!is_injectable_inner_verifier || branch.match->skip_transform))
                    {
                        needs_transform = false;
                    }
                    else if (auto* m_v_ptr = std::get_if<std::shared_ptr<MultiInjectVerifier>>(&branch.inner_verifier))
                    {
                        if (auto m_v = *m_v_ptr)
                        {
                            if (ctx.operator_binding_required)
                            {
                                final_verifier = m_v->binding_required;
                            }
                            else
                            {
                                final_verifier = ctx.transform_multi_verifier(m_v->multi_element);
                                needs_transform = false;
                            }
                        }
                    }

                    auto make_item = [&](const std::vector<SentinelKind>& accepted_for_item) -> ProcessingItem
                    {
                        std::string sentinel_tag = (accepted_for_item.size() == 1)
                                                       ? (" [" + to_string(accepted_for_item[0]) + "]")
                                                       : "";
                        std::vector<ContextStep> next_history = item.context_history;
                        next_history.push_back({
                            .context_name = ctx.name, .is_recursion = (next_rec_depth > item.recursion_depth)
                        });
                        return ProcessingItem{
                            .type = ctx.output_type,
                            .code = ctx.prefix + item.code + ctx.suffix,
                            .verifier = needs_transform ? ctx.transform_verifier(final_verifier) : final_verifier,
                            .path_name = item.path_name + " -> " + (next_rec_depth > item.recursion_depth
                                                                        ? std::string(ctx.name) + "(Recurse)"
                                                                        : std::string(ctx.name)) + sentinel_tag,
                            .cumulative_prefix = ctx.prefix + item.cumulative_prefix,
                            .depth = item.depth + 1,
                            .recursion_depth = next_rec_depth,
                            .skip_contexts = item.skip_contexts,
                            .is_skipped = branch.skip,
                            .context_overrides = item.context_overrides,
                            .custom_errors = branch.branch_errors,
                            .excluded_sentinels = branch.combined_excluded,
                            .accepted_sentinels = accepted_for_item,
                            .context_history = std::move(next_history)
                        };
                    };

                    if (branch.combined_accepted.size() <= 1)
                    {
                        return {make_item(branch.combined_accepted)};
                    }

                    std::vector<ProcessingItem> results;
                    for (size_t i = 0; i < branch.combined_accepted.size(); ++i)
                    {
                        results.push_back(make_item({branch.combined_accepted[i]}));
                    }
                    return results;
                };
            cb.on_block_branch = [inject_sentinels](const ProcessingItem& item, const Context& ctx, int next_rec_depth)
                -> std::vector<ProcessingItem>
                {
                    auto branch = resolve_branch_state(item, ctx);

                    bool is_injectable_inner_verifier = is_injectable_payload_for_context(
                        branch.inner_verifier, ctx.input_types);

                    auto make_item = [&](const std::vector<SentinelKind>& excluded_for_gen,
                                         const std::vector<SentinelKind>& accepted_for_gen,
                                         ModifierFilterMode post_mod_mode,
                                         size_t seed_offset,
                                         const std::string& path_suffix = "") -> ProcessingItem
                    {
                        std::vector<RecoveryBlock> pre, post;
                        std::string inner_code = item.code;
                        std::string inner_prefix = item.cumulative_prefix;

                        if (inject_sentinels)
                        {
                            size_t seed = DeterministicSampler::make_seed(item.path_name, ctx.name, item.code) +
                                seed_offset;
                            pre.push_back(
                                RecoverySentinel::generate_block_sentinel(
                                    seed, ctx.block_context, excluded_for_gen, {}));
                            post.push_back(RecoverySentinel::generate_block_sentinel(
                                seed + 1, ctx.block_context, excluded_for_gen, accepted_for_gen, post_mod_mode));
                            inner_code = pre[0].source + "\n  " + inner_code + "\n  " + post[0].source;
                            inner_prefix = pre[0].source + "\n  " + inner_prefix;
                        }

                        std::vector<ContextStep> next_history = item.context_history;
                        next_history.push_back({
                            .context_name = ctx.name, .is_recursion = (next_rec_depth > item.recursion_depth)
                        });
                        return ProcessingItem{
                            .type = ctx.output_type,
                            .code = ctx.prefix + inner_code + ctx.suffix,
                            .verifier = (branch.match && branch.match->verifier.has_value() && !is_injectable_inner_verifier)
                                            ? branch.inner_verifier
                                            : (ctx.transform_verifier_block
                                                   ? ctx.transform_verifier_block(branch.inner_verifier, pre, post)
                                                   : (ctx.transform_verifier
                                                          ? ctx.transform_verifier(branch.inner_verifier)
                                                          : branch.inner_verifier)),
                            .path_name = item.path_name + " -> " + (next_rec_depth > item.recursion_depth
                                                                        ? std::string(ctx.name) + "(Recurse)"
                                                                        : std::string(ctx.name)) + path_suffix,
                            .cumulative_prefix = ctx.prefix + inner_prefix,
                            .depth = item.depth + 1,
                            .recursion_depth = next_rec_depth,
                            .skip_contexts = item.skip_contexts,
                            .is_skipped = branch.skip,
                            .context_overrides = item.context_overrides,
                            .custom_errors = branch.branch_errors,
                            .excluded_sentinels = excluded_for_gen,
                            .accepted_sentinels = accepted_for_gen,
                            .context_history = std::move(next_history)
                        };
                    };

                    std::vector<ProcessingItem> results;
                    if (branch.combined_accepted.empty())
                    {
                        results.push_back(make_item(branch.combined_excluded, {}, ModifierFilterMode::UnmodifiedOnly, 0, ""));
                        if (inject_sentinels && RecoverySentinel::has_any_sentinel_with_modifier(ctx.block_context, branch.combined_excluded))
                        {
                            results.push_back(make_item(branch.combined_excluded, {}, ModifierFilterMode::ModifiedOnly, 13, " [with_modifier]"));
                        }
                        return results;
                    }

                    size_t step = 0;
                    for (size_t i = 0; i < branch.combined_accepted.size(); ++i)
                    {
                        SentinelKind k = branch.combined_accepted[i];
                        if (RecoverySentinel::is_sentinel_supported_in_block(ctx.block_context, k))
                        {
                            results.push_back(make_item(branch.combined_excluded, {k}, ModifierFilterMode::UnmodifiedOnly, (step++) * 31, ""));
                            if (inject_sentinels && RecoverySentinel::has_sentinel_with_modifier(ctx.block_context, k))
                            {
                                results.push_back(make_item(branch.combined_excluded, {k}, ModifierFilterMode::ModifiedOnly, (step++) * 31, " [with_modifier]"));
                            }
                        }
                    }

                    return results;
                };
            cb.should_abort = [] { return ::testing::Test::HasFailure(); };

            int start_depth = var_item.depth;
            int start_rec_depth = var_item.recursion_depth;
            ContextTreeWalker<ProcessingItem>::walk(std::move(var_item), start_depth, start_rec_depth, cb,
                                                    policy_override);
        }
    }
}
