#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class FunctionDefinitionRegressionTest : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(FunctionDefinitionRegressionTest, ParameterWithUnclosedGenericTypeFollowedByDefaultValueBinaryGreater)
    {
        ExpectParseErrorsWithRecovery(
            "func test(a: vector<int = a > b) -> void {}\n",
            {
                PErr{
                    .code = E::UnmatchedBracketAfterGenericArgs,
                    .line_start = 1, .column_start = 23,
                    .line_end = 1, .column_end = 24
                }
            },
            ProgramSpec{
                .functions = {
                    IsFunctionDef(
                        "test",
                        {},
                        {
                            ParamSpec{
                                .name = "a",
                                .type_v = IsType("vector", IsType("int")),
                                .default_v = IsBinary(TokenType::Greater, IsIdentifier("a"), IsIdentifier("b"))
                            }
                        },
                        {IsType("void")},
                        {}
                    )
                }
            }
        );
    }
}
