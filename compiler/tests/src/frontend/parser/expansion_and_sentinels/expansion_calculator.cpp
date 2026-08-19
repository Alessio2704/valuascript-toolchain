#include "expansion_calculator.h"
#include "context_tree_walker.h"
#include <algorithm>

namespace valuascript::compiler::test
{
    size_t ExpansionCalculator::compute_expected_expansions(InjectableType start_type,
                                                            const std::vector<std::string_view>& skip_contexts)
    {
        size_t count = 0;

        struct WalkState
        {
            InjectableType type;
            bool is_skipped;
        };

        ContextTreeWalker<WalkState>::Callbacks cb;

        cb.get_type = [](const WalkState& s) { return s.type; };

        cb.on_terminal = [&](WalkState s) { if (!s.is_skipped) count++; };

        cb.on_normal_branch = [&skip_contexts](const WalkState& s, const Context& ctx, int) -> std::vector<WalkState>
        {
            bool skip = s.is_skipped || std::find(skip_contexts.begin(), skip_contexts.end(), ctx.name) != skip_contexts
                .end();
            return { WalkState{.type = ctx.output_type, .is_skipped = skip} };
        };

        cb.on_block_branch = [&skip_contexts](const WalkState& s, const Context& ctx, int) -> std::vector<WalkState>
        {
            bool skip = s.is_skipped || std::find(skip_contexts.begin(), skip_contexts.end(), ctx.name) != skip_contexts
                .end();
            return { WalkState{.type = ctx.output_type, .is_skipped = skip} };
        };

        ContextTreeWalker<WalkState>::walk(WalkState{.type = start_type, .is_skipped = false}, 0, 0, cb);

        return count;
    }
}
