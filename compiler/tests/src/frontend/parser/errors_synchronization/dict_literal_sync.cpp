#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    class DictLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(DictLiteralParserSynchronizationTest, SynchronizesDictLiteralErrors)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        DictLiteralStressTests,
        DictLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_missing_closing_brace",
                .source_code = "let a = { x: 1 \nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 14} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                }
            }
        ),
        TestNameGenerator{}
    );
}
