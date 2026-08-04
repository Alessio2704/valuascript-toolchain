#pragma once

#include <functional>
#include <optional>
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/test_structures.h"
#include "expansion_policy.h"

namespace valuascript::compiler::test
{
    enum class StepKind { Promotion, Context };

    struct NextStep
    {
        StepKind kind;
        const Context* context = nullptr;
    };

    template <typename State>
    class ContextTreeWalker
    {
    public:
        struct Callbacks
        {
            std::function<InjectableType(const State&)> get_type;

            std::function<void(State)> on_terminal;
            std::function<void(const State&)> on_promotion;
            std::function<std::vector<State>(const State&, const Context&, int next_rec_depth)> on_normal_branch;
            std::function<std::vector<State>(const State&, const Context&, int next_rec_depth)> on_block_branch;

            std::function<std::vector<NextStep>(const std::vector<NextStep>&, int, int)> strategy = [](
                const std::vector<NextStep>& steps, int, int)
            {
                return steps;
            };
            std::function<bool()> should_abort = [] { return false; };
        };

        static void walk(State initial_state, int start_depth, int start_rec_depth, const Callbacks& callbacks,
                         std::optional<ExpansionPolicy> policy_override = std::nullopt)
        {
            auto policy = policy_override.value_or(ExpansionPolicy::current());
            walk_impl(std::move(initial_state), start_depth, start_rec_depth, policy.max_depth, policy.max_recursion,
                      callbacks);
        }

    private:
        static void walk_impl(State current, int depth, int rec_depth, int max_depth, int max_rec, const Callbacks& cb)
        {
            if (cb.should_abort()) return;

            InjectableType type = cb.get_type(current);

            if (is_terminal_type(type))
            {
                cb.on_terminal(std::move(current));
                return;
            }

            std::vector<NextStep> possible_steps;

            if (type == InjectableType::StrongStatement)
            {
                possible_steps.push_back({.kind = StepKind::Promotion, .context = nullptr});
            }

            if (depth < max_depth)
            {
                const auto& contexts = ContextRegistry::get_all_for(type);
                for (const auto& ctx : contexts)
                {
                    bool is_recursive = false;
                    for (auto ctx_input_type : ctx.input_types)
                    {
                        if (ctx_input_type == ctx.output_type)
                        {
                            is_recursive = true;
                            break;
                        }
                    }

                    if (is_recursive && rec_depth >= max_rec) continue;

                    possible_steps.push_back({.kind = StepKind::Context, .context = &ctx});
                }
            }

            auto chosen_steps = cb.strategy(possible_steps, depth, rec_depth);

            for (const auto& step : chosen_steps)
            {
                if (cb.should_abort()) return;

                if (step.kind == StepKind::Promotion)
                {
                    cb.on_promotion(current);
                }
                else if (step.context != nullptr)
                {
                    const auto& ctx = *step.context;
                    bool is_recursive = false;
                    for (auto ctx_input_type : ctx.input_types)
                    {
                        if (ctx_input_type == ctx.output_type)
                        {
                            is_recursive = true;
                            break;
                        }
                    }

                    int next_rec_depth = is_recursive ? rec_depth + 1 : rec_depth;

                    if (is_nested_block_context(ctx.block_context))
                    {
                        auto next_states = cb.on_block_branch(current, ctx, next_rec_depth);
                        for (auto& next_state : next_states)
                        {
                            if (cb.should_abort()) return;
                            walk_impl(std::move(next_state), depth + 1, next_rec_depth, max_depth, max_rec, cb);
                        }
                    }
                    else
                    {
                        auto next_states = cb.on_normal_branch(current, ctx, next_rec_depth);
                        for (auto& next_state : next_states)
                        {
                            if (cb.should_abort()) return;
                            walk_impl(std::move(next_state), depth + 1, next_rec_depth, max_depth, max_rec, cb);
                        }
                    }
                }
            }
        }
    };
}
