#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ExpressionRegressionsTest : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(ExpressionRegressionsTest, IfElseWithEnumMultipleCases)
    {
        ExpectParseErrorsWithRecovery(
            "@mod2() let a_m2 = 1\n"
            "\n"
            "\n"
            "let _test_expr = if 1 then 2 else enum Color: int { Red, Green, Blue }\n"
            "\n"
            "init()\n",
            {
                PErr{
                    .code = E::InvalidConstructPlacement,
                    .line_start = 4, .column_start = 35,
                    .line_end = 4, .column_end = 71
                }
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({AssignmentTargetSpec{.modifiers = {{"mod2", {}}}, .name = "a_m2"}}, IsNumber("1")),
                    IsAssignment({AssignmentTargetSpec{.name = "_test_expr"}}),
                    IsExprStmt(IsCall(IsIdentifier("init"), {}))
                }
            }
        );
    }
}
