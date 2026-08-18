#pragma once

#include "invalid_declaration_in_block_shared.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    struct InvalidDeclarationInExpressionTestCase
    {
        std::string test_name;
        Context context;
        InvalidDeclarationConstructCase construct_case;
    };

    template <typename Callback>
    inline void for_each_invalid_declaration_in_expression_program(const Context& ctx,
                                                                   const InvalidDeclarationConstructCase& construct,
                                                                   size_t seed,
                                                                   Callback&& callback)
    {
        std::string wrapped_code = "let _test_expr = " + ctx.prefix + construct.code + ctx.suffix + "\n";
        ProgramSpec inner_spec;
        SpecAdder::add(inner_spec, IsAssignment({AssignmentTargetSpec{.name = "_test_expr"}}));
        std::string inner_prefix = "let _test_expr = " + ctx.prefix;
        ParserTestBase::ForEachRecoveryProgram(wrapped_code, inner_spec, inner_prefix, seed, std::forward<Callback>(callback));
    }

    inline std::vector<Context> get_expression_test_contexts()
    {
        std::vector<Context> valid_ctxs;
        const auto& expression_ctxs = ContextRegistry::get_all_for(InjectableType::Expression);
        for (const auto& ctx : expression_ctxs)
        {
            if (ctx.output_type == InjectableType::Expression)
            {
                valid_ctxs.push_back(ctx);
            }
        }
        return valid_ctxs;
    }
}
