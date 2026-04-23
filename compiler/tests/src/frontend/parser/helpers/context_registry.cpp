#include "context_registry.h"
#include <algorithm>

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_all_for(const InjectableType type)
    {
        static std::vector<Context> cache = aggregate_all();
        std::vector<Context> result;
        for (const auto& ctx : cache)
        {
            if (std::find(ctx.input_types.begin(), ctx.input_types.end(), type) != ctx.input_types.end())
            {
                result.push_back(ctx);
            }
        }
        return result;
    }

    std::vector<Context> ContextRegistry::get_container_contexts_for(const InjectableType type)
    {
        std::vector<Context> result;
        for (const auto& ctx : get_all_for(type))
        {
            if (ctx.output_type == InjectableType::StrongStatement ||
                ctx.output_type == InjectableType::WeakStatement ||
                ctx.output_type == InjectableType::TopLevel)
            {
                result.push_back(ctx);
            }
        }
        return result;
    }

    std::vector<Context> ContextRegistry::get_block_contexts()
    {
        std::vector<Context> result;
        for (const auto& ctx : aggregate_all())
        {
            if (std::find(ctx.input_types.begin(), ctx.input_types.end(), InjectableType::StrongStatement) != ctx.
                input_types.end() ||
                std::find(ctx.input_types.begin(), ctx.input_types.end(), InjectableType::WeakStatement) != ctx.
                input_types.end())
            {
                if (ctx.output_type == InjectableType::TopLevel)
                {
                    result.push_back(ctx);
                }
            }
        }
        return result;
    }

    std::vector<Context> ContextRegistry::aggregate_all()
    {
        std::vector<Context> all;

        auto add = [&](const std::vector<Context>& list) { all.insert(all.end(), list.begin(), list.end()); };

        add(get_block_contexts_impl());
        add(get_expression_contexts());
        add(get_type_contexts());
        add(get_modifier_contexts());

        return all;
    }
}
