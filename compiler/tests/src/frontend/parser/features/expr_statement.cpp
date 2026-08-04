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
            auto reg = [](const ConstructCase<ExprStmtVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "SimpleCallStatement",
                .code = "init()",
                .verifier = IsExprStmt(IsCall(IsIdentifier("init"), {}))
            });

            reg({
                .name = "MultilineFormatting",
                .code = "my_function \n"
                "  ( \n"
                "    arg: 1 \n"
                "  )",
                .verifier = IsExprStmt(IsCall(IsIdentifier("my_function"), {{"arg", IsNumber("1")}}))
            });

            return true;
        }();
    }

    TEST_P(ExpressionStatementRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidExpressionStatement(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExpressionStatement,
        ExpressionStatementRegistryRunner,
        testing::ValuesIn(ConstructRegistry::expr_stmts()),
        TestNameGenerator{}
    );
}
