#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ExpressionStatementRegistryRunner : public ParserTestBase,
                                              public testing::WithParamInterface<RegistryEntry<ExprStmtVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("SimpleCallStatement",
                "init()",
                IsExprStmt(IsCall(IsIdentifier("init"), {})));

            reg("MultilineFormatting",
                "my_function \n"
                "  ( \n"
                "    arg: 1 \n"
                "  )",
                IsExprStmt(IsCall(IsIdentifier("my_function"), {{"arg", IsNumber("1")}})));

            return true;
        }();
    }

    TEST_P(ExpressionStatementRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidExpressionStatement(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExpressionStatement,
        ExpressionStatementRegistryRunner,
        testing::ValuesIn(ConstructRegistry::expr_stmts()),
        [](const testing::TestParamInfo<RegistryEntry<ExprStmtVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
