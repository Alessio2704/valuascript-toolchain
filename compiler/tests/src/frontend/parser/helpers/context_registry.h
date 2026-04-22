#pragma once
#include "context_infrastructure.h"

namespace valuascript::compiler::test
{
    class ContextRegistry
    {
    public:
        static std::vector<Context> get_all_for(InjectableType type);

    private:
        static std::vector<Context> aggregate_all();
        static std::vector<Context> get_top_level_contexts();
        static std::vector<Context> get_block_contexts();
        static std::vector<Context> get_expression_contexts();
        static std::vector<Context> get_type_contexts();
        static std::vector<Context> get_modifier_contexts();
    };
}
