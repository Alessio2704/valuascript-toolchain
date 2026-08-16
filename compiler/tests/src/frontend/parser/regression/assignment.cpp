#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentRegressionTest : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(AssignmentRegressionTest, UnclosedGenericTypeAnnotationFollowedByBinaryGreater)
    {
        ExpectParseErrorsWithRecovery(
            "let x: vector<int = a > b\n",
            {
                PErr{
                    .code = E::UnmatchedBracketAfterGenericArgs,
                    .line_start = 1, .column_start = 17,
                    .line_end = 1, .column_end = 18
                }
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec{
                                .name = "x",
                                .type_v = IsType("vector", IsType("int"))
                            }
                        },
                        IsBinary(TokenType::Greater, IsIdentifier("a"), IsIdentifier("b"))
                    )
                }
            }
        );
    }

    TEST_F(AssignmentRegressionTest, UnclosedFunctionCallInAssignmentSynchronizesToNextAssignment)
    {
        ExpectParseErrorsWithRecovery(
            "let a = func_call(\n\nlet b = some_other()",
            {
                PErr{
                    .code = E::ExpectedArgumentNameOrClosingParen,
                    .line_start = 1, .column_start = 19,
                    .line_end = 1, .column_end = 20
                }
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "a"}
                        },
                        IsCall(IsIdentifier("func_call"))
                    ),
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "b"}
                        },
                        IsCall(IsIdentifier("some_other"))
                    )
                }
            }
        );
    }

    TEST_F(AssignmentRegressionTest, MultipleAssignmentWithUnclosedFunctionCallAndTrailingSeparatorDirective)
    {
        ExpectParseErrorsWithRecovery(
            "#iterations = 10_000_\n"
            "\n"
            "// -- R&D Capitalization --\n"
            "let value_of_research_assets, current_year_amortization = get_rd(\n"
            "let a = 10\n",
            {
                PErr{
                    .code = LexerErrorCode::TrailingSeparatorInNumberLiteral,
                    .line_start = 1, .column_start = 15,
                    .line_end = 1, .column_end = 22
                },
                PErr{
                    .code = E::ExpectedArgumentNameOrClosingParen,
                    .line_start = 4, .column_start = 66,
                    .line_end = 4, .column_end = 67
                }
            },
            ProgramSpec{
                .directives = {
                    IsDirective("iterations", IsNumber("10_000_"))
                },
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "value_of_research_assets"},
                            AssignmentTargetSpec{.name = "current_year_amortization"}
                        },
                        IsCall(IsIdentifier("get_rd"))
                    ),
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "a"}
                        },
                        IsNumber("10")
                    )
                }
            }
        );
    }
}
