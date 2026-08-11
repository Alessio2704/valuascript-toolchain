#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    class FunctionBodyParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(FunctionBodyParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionBodyStressTests,
        FunctionBodyParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_closing_brace_escapes_to_top_level",
                .source_code = "func test() -> int {\n    let x = 1\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 3, .column = 9} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    ASSERT_EQ(ast.function_definitions[0]->body.size(), 2);
                    EXPECT_EQ(ast.execution_steps.size(), 0);
                }
            }
        ),
        TestNameGenerator{}
    );
}
