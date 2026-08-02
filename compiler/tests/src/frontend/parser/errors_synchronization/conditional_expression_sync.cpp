#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const ConditionalExpression* GetAssignedConditional(const Program& ast)
        {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected one assignment statement to survive.";
            if (ast.execution_steps.empty()) return nullptr;

            const auto* assign = dynamic_cast<const Assignment*>(ast.execution_steps.front().get());
            EXPECT_NE(assign, nullptr) << "Expected statement to be an Assignment.";
            if (!assign) return nullptr;

            const auto* cond = dynamic_cast<const ConditionalExpression*>(assign->value.get());
            EXPECT_NE(cond, nullptr) << "Expected assigned value to be a ConditionalExpression.";
            return cond;
        }
    }

    class ConditionalParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(ConditionalParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserExhaustiveStressTests,
        ConditionalParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "unterminated_grouping_in_condition_recovers_then_and_else",
                .source_code = "let a = if (x + 1 then 2 else 3\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 17} },
                .verify_ast = [](const Program& ast) {
                    const auto* cond = GetAssignedConditional(ast);
                    ASSERT_NE(cond, nullptr);

                    EXPECT_NE(cond->condition, nullptr);
                    auto* grouping = dynamic_cast<const GroupingExpression*>(cond->condition.get());
                    ASSERT_NE(grouping, nullptr);
                    EXPECT_NE(grouping->expression, nullptr) << "Grouping expression should preserve 'x + 1'";

                    EXPECT_NE(cond->then_branch, nullptr);
                    EXPECT_NE(cond->else_branch, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "unterminated_grouping_in_then_branch_recovers_else",
                .source_code = "let a = if x then (1 + 2 else 3\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 24} },
                .verify_ast = [](const Program& ast) {
                    const auto* cond = GetAssignedConditional(ast);
                    ASSERT_NE(cond, nullptr);

                    EXPECT_NE(cond->condition, nullptr);
                    EXPECT_NE(cond->then_branch, nullptr);

                    auto* grouping = dynamic_cast<const GroupingExpression*>(cond->then_branch.get());
                    ASSERT_NE(grouping, nullptr);
                    EXPECT_NE(grouping->expression, nullptr);

                    EXPECT_NE(cond->else_branch, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "unterminated_grouping_in_else_branch_bubbles_up_and_recovers_next_statement",
                .source_code = "let a = if x then 1 else (2 + 3\nlet b = 2\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 31},
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    const auto* assign = dynamic_cast<const Assignment*>(ast.execution_steps.front().get());
                    ASSERT_NE(assign, nullptr);
                    EXPECT_EQ(assign->targets[0].name, "a");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "bare_if_token_at_eof_discards_gracefully",
                .source_code = "let a = if\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 1, .column = 11},
                    {.code = Err::MissingThenToken, .line = 1, .column = 11},
                    {.code = Err::MissingElseToken, .line = 1, .column = 11},
                },
                .verify_ast = [](const Program& ast) {
                    const auto* cond = GetAssignedConditional(ast);
                    ASSERT_NE(cond, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "nested_if_as_condition_missing_parenthesis_recovers_outer_branches",
                .source_code = "let a = if (if x then 1 else 2 then 3 else 4\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 30} },
                .verify_ast = [](const Program& ast) {
                    const auto* cond = GetAssignedConditional(ast);
                    ASSERT_NE(cond, nullptr);

                    EXPECT_NE(cond->condition, nullptr);
                    auto* grouping = dynamic_cast<const GroupingExpression*>(cond->condition.get());
                    ASSERT_NE(grouping, nullptr);

                    auto* inner = dynamic_cast<const ConditionalExpression*>(grouping->expression.get());
                    ASSERT_NE(inner, nullptr);
                    EXPECT_NE(inner->condition, nullptr);
                    EXPECT_NE(inner->then_branch, nullptr);
                    EXPECT_NE(inner->else_branch, nullptr);

                    EXPECT_NE(cond->then_branch, nullptr);
                    EXPECT_NE(cond->else_branch, nullptr);
                }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
