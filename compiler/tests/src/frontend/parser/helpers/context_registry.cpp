#include "context_registry.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_all_for(const InjectableType type)
    {
        static std::vector<Context> cache = aggregate_all();
        std::vector<Context> result;
        for (const auto& ctx : cache)
        {
            if (std::find(ctx.allowed_atoms.begin(), ctx.allowed_atoms.end(), type) != ctx.allowed_atoms.end())
            {
                result.push_back(ctx);
            }
        }
        return result;
    }

    std::vector<Context> ContextRegistry::aggregate_all()
    {
        std::vector<Context> all;

        auto add = [&](const std::vector<Context>& list) { all.insert(all.end(), list.begin(), list.end()); };

        add(get_top_level_contexts());
        add(get_block_contexts());
        add(get_expression_contexts());
        add(get_type_contexts());
        add(get_modifier_contexts());

        return all;
    }
}
