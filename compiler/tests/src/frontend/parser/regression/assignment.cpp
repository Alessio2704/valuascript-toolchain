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
}
