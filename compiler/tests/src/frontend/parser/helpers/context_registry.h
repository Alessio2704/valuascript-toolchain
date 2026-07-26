#pragma once
#include "context_infrastructure.h"

namespace valuascript::compiler::test
{
    class ContextRegistry
    {
    public:
        static const std::vector<Context>& get_all_for(InjectableType type);
        static const std::vector<Context>& get_container_contexts_for(InjectableType type);
        static const std::vector<Context>& get_block_contexts();

    private:
        static const std::vector<Context>& aggregate_all();
        static const std::vector<Context>& get_expression_contexts();
        static const std::vector<Context>& get_type_contexts();
        static const std::vector<Context>& get_modifier_contexts();
        static const std::vector<Context>& get_block_contexts_impl();
        static const std::vector<Context>& get_extension_contexts();
        static const std::vector<Context>& get_identifier_contexts();
    };
}
