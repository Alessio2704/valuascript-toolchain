#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    class GroupingParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(GroupingParserSynchronizationTest, SynchronizesGroupingErrors) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        GroupingStressTests,
        GroupingParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_recursive_unclosed",
                .source_code = "let a = (((1 + 2\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 16},
                    {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 16},
                    {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 16}
                },
                .verify_ast = [](const Program& ast) {
                    auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto* g1 = dynamic_cast<GroupingExpression*>(assign->value.get());
                    auto* g2 = dynamic_cast<GroupingExpression*>(g1->expression.get());
                    auto* g3 = dynamic_cast<GroupingExpression*>(g2->expression.get());
                    ASSERT_NE(g3, nullptr);
                }
            }
        ),
        TestNameGenerator{}
    );
}
