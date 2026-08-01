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
            "no_left_paren_func_empty_ast",
            "func test a: int) -> int {}\n"
            "let a = 1\n",
            {{Err::ExpectedLeftParenAfterFunctionName, 1, 11}},
            ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_paren_1_func_empty_ast",
            "func test(a: int -> int {}\n"
            "let a = 1\n",
            {{Err::ExpectedRightParenAfterParameters, 1, 16}},
            ExpectOneFunction()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_paren_2_func_empty_ast",
            "func test(a: int { return 1 }\n"
            "let a = 1\n",
            {{Err::ExpectedRightParenAfterParameters, 1, 16}, {Err::MissingArrowInFunction, 1, 18}},
            ExpectOneFunction()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_left_brace_func_empty_ast",
            "func test(a: int) -> int  return 1 }\n"
            "let a = 1\n",
            {{Err::ExpectedLeftBraceBeforeFunctionBody, 1, 27}},
            ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_brace_func",
            "func test(a: int) -> int  { return 1 \n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 1, 36}},
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->body.size(), 1);
            ASSERT_EQ(ast.execution_steps.size(), 0);
            },
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_brace_but_valid_body_after",
            "func test(a: int) -> int  { return 1 \n"
            "let a = 1\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 9}},
            [](const Program& ast) {
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
