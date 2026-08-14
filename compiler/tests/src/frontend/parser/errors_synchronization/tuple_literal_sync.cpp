#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    class TupleLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TupleLiteralParserSynchronizationTest, SynchronizesTupleLiteralErrors) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TupleLiteralStressTests,
        TupleLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_deep_unclosed_cascade",
                .source_code = "let a = (1, (2, (3, (4, \n",
                .expected_errors = {
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 23},
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 23},
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 23},
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 23}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 1);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(assign, nullptr);

                    auto tuple = dynamic_cast<TupleLiteral*>(assign->value.get());
                    ASSERT_NE(tuple, nullptr);
                    ASSERT_EQ(tuple->elements.size(), 2);
                },
            }
        ),
        TestNameGenerator{}
    );
}
