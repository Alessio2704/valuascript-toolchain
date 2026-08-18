#pragma once

#include "invalid_declaration_in_expression_shared.h"

namespace valuascript::compiler::test
{
    class InvalidDeclarationInExpressionConstructRegistry
    {
    public:
        static const std::vector<InvalidDeclarationConstructCase>& cases();
        static std::vector<InvalidDeclarationConstructCase> cases_for_context(const Context& ctx);
    };

    bool should_test_construct_in_context_expr(const Context& ctx, const InvalidDeclarationConstructCase& construct);
    std::vector<InvalidDeclarationInExpressionTestCase> GenerateInvalidDeclarationInExpressionTestCases();
}
