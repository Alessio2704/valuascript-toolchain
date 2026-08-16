#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ExpressionStatementErrorRegistryRunner : public ParserTestBase,
                                                   public testing::WithParamInterface<ErrorRegistryEntry<ExprStmtVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprStmtVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "ExprStatementCallMissingClosingParen",
                .code = "f(a: 1",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightParenAfterArguments,
                        .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7
                    }
                },
                .verifier = IsExprStmt(IsCall(
                    IsIdentifier("f"), {
                        {.label = "a", .value_v = IsNumber("1")}
                    }
                )),
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "InvalidStandaloneStatement1",
                .code = "x + y",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 6}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement2",
                .code = "a[0] + y",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 9}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement3",
                .code = "a[0].a + y",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 11}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement4",
                .code = "a",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 2}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement5",
                .code = "a[0]",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement6",
                .code = "{} + a",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 7}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement7",
                .code = "-1",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 3}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement8",
                .code = "1 + 2",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 6}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement9",
                .code = "true",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5}},
                .verifier = IsNull()
            });


            return true;
        }();
    }

    TEST_P(ExpressionStatementErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
            accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectExpressionStatementErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
                                        accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExpressionStatement,
        ExpressionStatementErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::expr_stmts()),
        TestNameGenerator{}
    );
}
