#include "expansion_calculator.h"
#include "context_tree_walker.h"

namespace valuascript::compiler::test
{
    size_t ExpansionCalculator::compute_expected_expansions(InjectableType start_type)
    {
        size_t count = 0;

        ContextTreeWalker<InjectableType>::Callbacks cb;

        cb.get_type = [](const InjectableType& type) { return type; };

        cb.on_terminal = [&](InjectableType) { count++; };

        cb.on_promotion = [&](const InjectableType&) { count++; };

        cb.on_normal_branch = [](const InjectableType&, const Context& ctx, int)
        {
            return ctx.output_type;
        };

        cb.on_block_branch = [](const InjectableType&, const Context& ctx, int)
        {
            return ctx.output_type;
        };

        ContextTreeWalker<InjectableType>::walk(start_type, 0, 0, cb);

        return count;
    }
}
