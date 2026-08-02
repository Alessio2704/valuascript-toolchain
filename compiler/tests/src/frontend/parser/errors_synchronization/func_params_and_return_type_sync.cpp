#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        auto ExpectNoFunctions()
        {
            return [](const Program& ast)
            {
                ASSERT_EQ(ast.function_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }

        auto ExpectOneFunction()
        {
            return [](const Program& ast)
            {
                ASSERT_EQ(ast.function_definitions.size(), 1);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }
    }

    class FunctionParametersAndReturnTypeParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(FunctionParametersAndReturnTypeParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionParametersAndReturnTypeStressTest,
        FunctionParametersAndReturnTypeParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "no_left_paren_func_empty_ast",
                .source_code = "func test a: int) -> int {}\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedLeftParenAfterFunctionName, .line = 1, .column = 11} },
                .verify_ast = ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_right_paren_1_func_empty_ast",
                .source_code = "func test(a: int -> int {}\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterParameters, .line = 1, .column = 16} },
                .verify_ast = ExpectOneFunction()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_right_paren_2_func_empty_ast",
                .source_code = "func test(a: int { return 1 }\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterParameters, .line = 1, .column = 16}, {.code = Err::MissingArrowInFunction, .line = 1, .column = 18} },
                .verify_ast = ExpectOneFunction()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_left_brace_func_empty_ast",
                .source_code = "func test(a: int) -> int  return 1 }\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedLeftBraceBeforeFunctionBody, .line = 1, .column = 27} },
                .verify_ast = ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_right_brace_func",
                .source_code = "func test(a: int) -> int  { return 1 \n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 1, .column = 36} },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    ASSERT_EQ(ast.function_definitions[0]->body.size(), 1);
                    ASSERT_EQ(ast.execution_steps.size(), 0);
                },
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_right_brace_but_valid_body_after",
                .source_code = "func test(a: int) -> int  { return 1 \nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 9} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->body.size(), 2);
                    EXPECT_EQ(ast.execution_steps.size(), 0);
                },
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase> &test_info) {
        return test_info.param.test_name;
        }
    );
}
