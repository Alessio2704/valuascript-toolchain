#include "context_registry.h"
#include <unordered_map>

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_all_for(const InjectableType type)
    {
        static const auto map = []()
        {
            std::unordered_map<InjectableType, std::vector<Context>> m;
            const auto& cache = aggregate_all();
            for (const auto& ctx : cache)
            {
                for (const auto& in_type : ctx.input_types)
                {
                    m[in_type].push_back(ctx);
                }
            }
            return m;
        }();

        static const std::vector<Context> empty;
        auto it = map.find(type);
        return (it != map.end()) ? it->second : empty;
    }

    const std::vector<Context>& ContextRegistry::get_container_contexts_for(const InjectableType type)
    {
        static const auto map = []()
        {
            std::unordered_map<InjectableType, std::vector<Context>> m;
            for (const auto& ctx : aggregate_all())
            {
                if (ctx.output_type == InjectableType::StrongStatement ||
                    ctx.output_type == InjectableType::WeakStatement ||
                    ctx.output_type == InjectableType::TopLevel)
                {
                    for (const auto& in_type : ctx.input_types)
                    {
                        m[in_type].push_back(ctx);
                    }
                }
            }
            return m;
        }();

        static const std::vector<Context> empty;
        auto it = map.find(type);
        return (it != map.end()) ? it->second : empty;
    }

    const std::vector<Context>& ContextRegistry::get_block_contexts()
    {
        static const std::vector<Context> cached = []()
        {
            std::vector<Context> result;
            for (const auto& ctx : aggregate_all())
            {
                if (is_nested_block_context(ctx.block_context))
                {
                    result.push_back(ctx);
                }
            }
            return result;
        }();
        return cached;
    }

    const std::vector<std::string_view>& ContextRegistry::get_nested_expression_context_names()
    {
        static const std::vector<std::string_view> cached = []()
        {
            std::vector<std::string_view> result;
            for (const auto& ctx : get_expression_contexts())
            {
                bool has_trailing_suffix = !ctx.suffix.empty() &&
                    ctx.suffix.find_first_not_of(" \t\r\n") != std::string_view::npos;
                if (has_trailing_suffix ||
                    (ctx.output_type != InjectableType::StrongStatement &&
                     ctx.output_type != InjectableType::WeakStatement &&
                     ctx.output_type != InjectableType::TopLevel))
                {
                    result.push_back(ctx.name);
                }
            }
            return result;
        }();
        return cached;
    }

    const std::vector<Context>& ContextRegistry::aggregate_all()
    {
        static const std::vector<Context> all = []()
        {
            std::vector<Context> vec;
            auto add = [&](const std::vector<Context>& list)
            {
                vec.insert(vec.end(), list.begin(), list.end());
            };

            add(get_block_contexts_impl());
            add(get_expression_contexts());
            add(get_type_contexts());
            add(get_modifier_contexts());
            add(get_identifier_contexts());
            add(get_extension_contexts());
            add(get_top_level_contexts());

            return vec;
        }();
        return all;
    }
}
