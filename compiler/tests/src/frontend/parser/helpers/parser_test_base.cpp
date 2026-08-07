#include "parser_test_base.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "context_registry.h"
#include "error_shifter.h"
#include "recovery_sentinel.h"
#include "frontend/lexer/lexer_stage.h"
#include "frontend/parser/parser_stage.h"
#include "../expansion_and_sentinels/context_tree_walker.h"
#include "modifier_context_augmentation_manager.h"

namespace valuascript::compiler::test
{
    std::string ParserTestBase::format_source_with_lines(const std::string& code)
    {
        std::ostringstream oss;
        oss << "\n--- Full Source Code Listing ---\n";
        std::istringstream stream(code);
        std::string line;
        int line_num = 1;
        while (std::getline(stream, line))
        {
            oss << std::setw(3) << line_num++ << " | " << line << "\n";
        }
        oss << "--------------------------------\n";
        return oss.str();
    }

    std::shared_ptr<Program> ParserTestBase::run_parser(const std::string& code, CompilerContext& context)
    {
        thread_local LexerStage lexer;
        thread_local ParserStage parser;

        std::vector<CompilerStageArtifact> artifacts;
        artifacts.reserve(3);
        artifacts.emplace_back(CompilerStageArtifactCode::SourceCode, code);
        artifacts.emplace_back(CompilerStageArtifactCode::FilePath, std::string("test_script.vs"));

        artifacts.push_back(lexer.run(context, artifacts));

        CompilerStageArtifact ast_artifact = parser.run(context, artifacts);
        return extract_artifact_data<std::shared_ptr<Program>>({ast_artifact}, CompilerStageArtifactCode::Ast);
    }

    std::vector<ProcessingItem> ParserTestBase::apply_context_augmentations(
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
        ProcessingItem base_item{type, snippet, verifier, group_name, "", 0, 0, skip_contexts, false, context_overrides, std::nullopt, excluded_sentinels, accepted_sentinels};

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

    void ParserTestBase::expand_to_top_level_stream(std::vector<ProcessingItem> items,
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
                bool skip = item.is_skipped || std::find(item.skip_contexts.begin(), item.skip_contexts.end(), ctx.name)
                    != item.skip_contexts.end();

                const ContextOverrideAny* match = nullptr;
                for (const auto& ov : item.context_overrides)
                {
                    if (ov.context_name == ctx.name)
                    {
                        match = &ov;
                        break;
                    }
                }

                const ContextOverrideAny* verifier_match = (item.depth == 0) ? match : nullptr;

                UniversalVerifier inner_verifier = (verifier_match && verifier_match->verifier.has_value()) ? verifier_match->verifier.value() : item.verifier;
                std::optional<std::vector<ParserExpectedError>> branch_errors = (verifier_match && verifier_match->errors.has_value()) ? verifier_match->errors : item.custom_errors;

                std::vector<SentinelKind> combined_excluded = item.excluded_sentinels;
                if (match && !match->excluded_sentinels.empty())
                {
                    for (auto k : match->excluded_sentinels)
                    {
                        if (std::find(combined_excluded.begin(), combined_excluded.end(), k) == combined_excluded.end())
                        {
                            combined_excluded.push_back(k);
                        }
                    }
                }

                std::vector<SentinelKind> combined_accepted = item.accepted_sentinels;
                if (match && !match->accepted_sentinels.empty())
                {
                    combined_accepted = match->accepted_sentinels;
                }

                if (!combined_accepted.empty() && !combined_excluded.empty())
                {
                    std::vector<SentinelKind> filtered;
                    for (auto k : combined_accepted)
                    {
                        if (std::find(combined_excluded.begin(), combined_excluded.end(), k) == combined_excluded.end())
                        {
                            filtered.push_back(k);
                        }
                    }
                    combined_accepted = std::move(filtered);
                }

                UniversalVerifier final_verifier = inner_verifier;
                bool needs_transform = true;

                bool is_injectable_inner_verifier = is_injectable_payload_for_context(inner_verifier, ctx.input_types);

                if (match && match->verifier.has_value())
                {
                    needs_transform = is_injectable_inner_verifier && (ctx.transform_verifier != nullptr);
                }
                else if (auto* m_v_ptr = std::get_if<std::shared_ptr<MultiInjectVerifier>>(&inner_verifier))
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
                    std::string sentinel_tag = (accepted_for_item.size() == 1) ? (" [" + to_string(accepted_for_item[0]) + "]") : "";
                    std::vector<ContextStep> next_history = item.context_history;
                    next_history.push_back({.context_name = ctx.name, .is_recursion = (next_rec_depth > item.recursion_depth)});
                    return ProcessingItem{
                        ctx.output_type, ctx.prefix + item.code + ctx.suffix,
                        needs_transform ? ctx.transform_verifier(final_verifier) : final_verifier,
                        item.path_name + " -> " + (next_rec_depth > item.recursion_depth
                                                       ? std::string(ctx.name) + "(Recurse)"
                                                       : std::string(ctx.name)) + sentinel_tag,
                        ctx.prefix + item.cumulative_prefix, item.depth + 1, next_rec_depth,
                        item.skip_contexts, skip, item.context_overrides, branch_errors,
                        combined_excluded, accepted_for_item, std::move(next_history)
                    };
                };

                if (combined_accepted.size() <= 1)
                {
                    return { make_item(combined_accepted) };
                }

                std::vector<ProcessingItem> results;
                for (size_t i = 0; i < combined_accepted.size(); ++i)
                {
                    results.push_back(make_item({ combined_accepted[i] }));
                }
                return results;
            };
            cb.on_block_branch = [inject_sentinels](const ProcessingItem& item, const Context& ctx, int next_rec_depth)
            -> std::vector<ProcessingItem>
            {
                bool skip = item.is_skipped || std::find(item.skip_contexts.begin(), item.skip_contexts.end(), ctx.name)
                    != item.skip_contexts.end();

                const ContextOverrideAny* match = nullptr;
                for (const auto& ov : item.context_overrides)
                {
                    if (ov.context_name == ctx.name)
                    {
                        match = &ov;
                        break;
                    }
                }

                const ContextOverrideAny* verifier_match = (item.depth == 0) ? match : nullptr;

                UniversalVerifier inner_verifier = (verifier_match && verifier_match->verifier.has_value()) ? verifier_match->verifier.value() : item.verifier;
                std::optional<std::vector<ParserExpectedError>> branch_errors = (verifier_match && verifier_match->errors.has_value()) ? verifier_match->errors : item.custom_errors;

                std::vector<SentinelKind> combined_excluded = item.excluded_sentinels;
                if (match && !match->excluded_sentinels.empty())
                {
                    for (auto k : match->excluded_sentinels)
                    {
                        if (std::find(combined_excluded.begin(), combined_excluded.end(), k) == combined_excluded.end())
                        {
                            combined_excluded.push_back(k);
                        }
                    }
                }

                std::vector<SentinelKind> combined_accepted = item.accepted_sentinels;
                if (match && !match->accepted_sentinels.empty())
                {
                    combined_accepted = match->accepted_sentinels;
                }

                if (!combined_accepted.empty() && !combined_excluded.empty())
                {
                    std::vector<SentinelKind> filtered;
                    for (auto k : combined_accepted)
                    {
                        if (std::find(combined_excluded.begin(), combined_excluded.end(), k) == combined_excluded.end())
                        {
                            filtered.push_back(k);
                        }
                    }
                    combined_accepted = std::move(filtered);
                }

                bool is_injectable_inner_verifier = is_injectable_payload_for_context(inner_verifier, ctx.input_types);

                auto make_item = [&](const std::vector<SentinelKind>& excluded_for_gen,
                                     const std::vector<SentinelKind>& accepted_for_gen,
                                     size_t seed_offset) -> ProcessingItem
                {
                    std::vector<RecoveryBlock> pre, post;
                    std::string inner_code = item.code;
                    std::string inner_prefix = item.cumulative_prefix;

                    if (inject_sentinels)
                    {
                        size_t seed = DeterministicSampler::make_seed(item.path_name, ctx.name, item.code) + seed_offset;
                        pre.push_back(RecoverySentinel::generate_block_sentinel(seed, ctx.block_context, excluded_for_gen, {}));
                        post.push_back(RecoverySentinel::generate_block_sentinel(seed + 1, ctx.block_context, excluded_for_gen, accepted_for_gen));
                        inner_code = pre[0].source + "\n  " + inner_code + "\n  " + post[0].source;
                        inner_prefix = pre[0].source + "\n  " + inner_prefix;
                    }

                    std::vector<ContextStep> next_history = item.context_history;
                    next_history.push_back({.context_name = ctx.name, .is_recursion = (next_rec_depth > item.recursion_depth)});
                    return ProcessingItem{
                        ctx.output_type, ctx.prefix + inner_code + ctx.suffix,
                        (match && match->verifier.has_value() && !is_injectable_inner_verifier)
                            ? inner_verifier
                            : (ctx.transform_verifier_block ? ctx.transform_verifier_block(inner_verifier, pre, post)
                                                            : (ctx.transform_verifier ? ctx.transform_verifier(inner_verifier) : inner_verifier)),
                        item.path_name + " -> " + (next_rec_depth > item.recursion_depth
                                                       ? std::string(ctx.name) + "(Recurse)"
                                                       : std::string(ctx.name)),
                        ctx.prefix + inner_prefix, item.depth + 1, next_rec_depth,
                        item.skip_contexts, skip, item.context_overrides, branch_errors,
                        excluded_for_gen, accepted_for_gen, std::move(next_history)
                    };
                };

                if (combined_accepted.empty())
                {
                    return { make_item(combined_excluded, {}, 0) };
                }

                std::vector<ProcessingItem> results;
                for (size_t i = 0; i < combined_accepted.size(); ++i)
                {
                    SentinelKind k = combined_accepted[i];
                    if (RecoverySentinel::is_sentinel_supported_in_block(ctx.block_context, k))
                    {
                        results.push_back(make_item(combined_excluded, { k }, i * 31));
                    }
                }

                return results;
            };
            cb.should_abort = [] { return HasFailure(); };

            int start_depth = var_item.depth;
            int start_rec_depth = var_item.recursion_depth;
            ContextTreeWalker<ProcessingItem>::walk(std::move(var_item), start_depth, start_rec_depth, cb,
                                                    policy_override);
        }
    }

    void ParserTestBase::ExpectValidUnified(InjectableType type, std::vector<ProcessingItem> items,
                                            const std::string& group_name)
    {
        size_t actual_expansions = 0;

        expand_to_top_level_stream(std::move(items), [&](ProcessingItem&& item)
        {
            actual_expansions++;
            ProgramSpec spec;
            std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, item.verifier);
            SCOPED_TRACE("Context: " + item.path_name);
            ExpectValidParse(item.code, spec);
        }, false);

        if (!HasFailure() && !items.empty())
        {
            size_t expected_expansions = ExpansionCalculator::compute_expected_expansions(type, items[0].skip_contexts);
            EXPECT_EQ(actual_expansions, expected_expansions * items.size()) << "Expansion count mismatch for " <<
                group_name << " (Valid Parse).";
        }
    }

    void ParserTestBase::ExpectParseErrorsUnified(InjectableType type, std::vector<ProcessingItem> items,
                                                  const std::vector<ParserExpectedError>& errors,
                                                  const std::string& group_name)
    {
        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = DeterministicSampler::make_seed(test_info ? test_info->name() : "fallback");

        size_t actual_expansions = 0;
        size_t scenario_index = 0;

        expand_to_top_level_stream(std::move(items), [&](ProcessingItem&& item)
        {
            actual_expansions++;
            RunRecoveryScenario(std::move(item), errors, base_seed + (scenario_index++ * 2));
        }, true);

        if (!HasFailure() && !items.empty())
        {
            size_t expected_expansions = ExpansionCalculator::compute_expected_expansions(type, items[0].skip_contexts);
            EXPECT_EQ(actual_expansions, expected_expansions * items.size()) << "Expansion count mismatch for " <<
                group_name << " (Error Recovery).";
        }
    }


    std::vector<ConstructedRecoveryProgram> ParserTestBase::BuildRecoveryPrograms(
        std::string inner_code,
        ProgramSpec inner_spec,
        const std::string& inner_prefix,
        size_t seed,
        const std::vector<SentinelKind>& excluded_sentinels,
        const std::vector<SentinelKind>& accepted_sentinels,
        const std::string& path_name)
    {
        auto build_single = [&](size_t s, const std::vector<SentinelKind>& acc, const std::string& path_suffix) -> ConstructedRecoveryProgram
        {
            RecoveryBlock pre = RecoverySentinel::generate_block_sentinel(s, BlockContext::TopLevel, excluded_sentinels, {});
            RecoveryBlock post = RecoverySentinel::generate_block_sentinel(s + 1, BlockContext::TopLevel, excluded_sentinels, acc);

            std::string code = inner_code;
            while (!code.empty() && (code.back() == '\n' || code.back() == '\r'))
            {
                code.pop_back();
            }

            std::string full_code = pre.source + "\n\n" + code + "\n\n" + post.source + "\n";
            std::string prefix_for_shifting = pre.source + "\n\n" + inner_prefix;

            ProgramSpec full_spec;
            if (pre.add_to_spec) pre.add_to_spec(full_spec);
            full_spec = MergeSpecs(std::move(full_spec), inner_spec);
            if (post.add_to_spec) post.add_to_spec(full_spec);

            std::string full_path = path_name.empty() ? "" : (path_name + path_suffix);
            return {std::move(full_code), std::move(full_spec), std::move(prefix_for_shifting), std::move(full_path)};
        };

        if (accepted_sentinels.size() > 1)
        {
            std::vector<ConstructedRecoveryProgram> results;
            results.reserve(accepted_sentinels.size());
            for (size_t i = 0; i < accepted_sentinels.size(); ++i)
            {
                std::string tag = " [" + to_string(accepted_sentinels[i]) + "]";
                results.push_back(build_single(seed + (i * 2), { accepted_sentinels[i] }, tag));
            }
            return results;
        }

        return { build_single(seed, accepted_sentinels, "") };
    }

    std::vector<ConstructedRecoveryProgram> ParserTestBase::BuildRecoveryPrograms(
        const ProcessingItem& item,
        size_t seed,
        std::optional<ProgramSpec> inner_spec_override)
    {
        ProgramSpec spec;
        if (inner_spec_override.has_value())
        {
            spec = std::move(inner_spec_override.value());
        }
        else
        {
            std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, item.verifier);
        }

        return BuildRecoveryPrograms(
            item.code,
            std::move(spec),
            item.cumulative_prefix,
            seed,
            item.excluded_sentinels,
            item.accepted_sentinels,
            item.path_name
        );
    }

    void ParserTestBase::RunRecoveryScenario(ProcessingItem&& item,
                                             const std::vector<ParserExpectedError>& errors,
                                             size_t seed)
    {
        ForEachRecoveryProgram(item, seed, [&](const ConstructedRecoveryProgram& prog)
        {
            const auto& errors_to_use = item.custom_errors.has_value() ? item.custom_errors.value() : errors;
            auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, errors_to_use);
            ExpectParseErrors(prog.full_code, shifted, prog.full_spec);
        });
    }

    void ParserTestBase::ExpectValidParse(const std::string& code, const ProgramSpec& spec)
    {
        SCOPED_TRACE(format_source_with_lines(code));
        CompilerContext context;
        context.settings.fail_fast = false;
        std::shared_ptr<Program> ast = run_parser(code, context);

        const auto& errors = context.diagnostics.get_errors();
        if (!errors.empty())
        {
            ADD_FAILURE() << "Expected no errors, but got " << errors.size() << ". First: " << errors[0].what();
        }

        ASSERT_NE(ast, nullptr) << "Parser returned null AST.";
        ExpectProgram(ast.get(), spec);
    }

    void ParserTestBase::ExpectParseErrors(const std::string& code,
                                           const std::vector<ParserExpectedError>& expected_errors,
                                           const std::optional<ProgramSpec>& spec)
    {
        SCOPED_TRACE(format_source_with_lines(code));
        CompilerContext context;
        context.settings.fail_fast = false;
        std::shared_ptr<Program> ast = run_parser(code, context);

        const auto& actual_errors = context.diagnostics.get_errors();
        if (actual_errors.size() != expected_errors.size()) {
            for (const auto& err : actual_errors) {
                std::cout << "ACTUAL ERR: code=" << err.get_code().index() << " msg=" << err.what() << " span=" << err.get_span().line_start << ":" << err.get_span().column_start << "-" << err.get_span().line_end << ":" << err.get_span().column_end << std::endl;
            }
        }
        ASSERT_EQ(actual_errors.size(), expected_errors.size()) << "Error count mismatch.";

        for (size_t i = 0; i < actual_errors.size(); ++i)
        {
            EXPECT_EQ(actual_errors[i].get_code(), expected_errors[i].code);
            if (!expected_errors[i].skip_span_check)
            {
                EXPECT_EQ(actual_errors[i].get_span().line_start, expected_errors[i].line_start) <<
 "Line start mismatch on error: " << i << std::endl;
                EXPECT_EQ(actual_errors[i].get_span().line_end, expected_errors[i].line_end) <<
 "Line end mismatch on error: " << i << std::endl;
                EXPECT_EQ(actual_errors[i].get_span().column_start, expected_errors[i].column_start) <<
 "Column start mismatch on error: " << i << std::endl;
                EXPECT_EQ(actual_errors[i].get_span().column_end, expected_errors[i].column_end) <<
 "Column end mismatch on error: " << i << std::endl;
            }
        }

        if (spec.has_value())
        {
            ASSERT_NE(ast, nullptr);
            ExpectProgram(ast.get(), spec.value());
        }
    }

    void ParserTestBase::ExpectParseErrorsWithRecovery(const std::string& code,
                                                       const std::vector<ParserExpectedError>& expected_errors,
                                                       ProgramSpec broken_part_spec)
    {
        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = DeterministicSampler::make_seed(test_info ? test_info->name() : "fallback");

        ForEachRecoveryProgram(code, std::move(broken_part_spec), "", base_seed, [&](const ConstructedRecoveryProgram& prog)
        {
            auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, expected_errors);
            ExpectParseErrors(prog.full_code, shifted, prog.full_spec);
        });
    }
}
