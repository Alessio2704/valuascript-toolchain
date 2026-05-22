#pragma once

#include <functional>
#include "context_registry.h"
#include "test_structures.h"
#include "expansion_policy.h"

namespace valuascript::compiler::test
{
    template <typename State>
    class ContextTreeWalker
    {
    public:
        struct Callbacks
        {
            std::function<InjectableType(const State&)> get_type;

            std::function<void(State)> on_terminal;
            std::function<void(const State&)> on_promotion;
            std::function<State(const State&, const Context&, int next_rec_depth)> on_normal_branch;
            std::function<State(const State&, const Context&, int next_rec_depth)> on_block_branch;

            std::function<bool()> should_abort = [] { return false; };
        };

        static void walk(State initial_state, int start_depth, int start_rec_depth, const Callbacks& callbacks)
        {
            auto policy = ExpansionPolicy::current();
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

            if (type == InjectableType::StrongStatement)
            {
                cb.on_promotion(current);
                if (cb.should_abort()) return;
            }

            if (depth >= max_depth) return;

            auto contexts = ContextRegistry::get_all_for(type);
            for (const auto& ctx : contexts)
            {
                if (cb.should_abort()) return;

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

                int next_rec_depth = is_recursive ? rec_depth + 1 : rec_depth;

                State next_state;
                if (ctx.is_block_context)
                {
                    next_state = cb.on_block_branch(current, ctx, next_rec_depth);
                }
                else
                {
                    next_state = cb.on_normal_branch(current, ctx, next_rec_depth);
                }

                walk_impl(std::move(next_state), depth + 1, next_rec_depth, max_depth, max_rec, cb);
            }
        }
    };
}
