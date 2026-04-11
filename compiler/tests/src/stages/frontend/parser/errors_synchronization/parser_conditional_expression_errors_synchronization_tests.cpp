#include <gtest/gtest.h>
#include "frontend/parser/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const ConditionalExpression *GetAssignedConditional(const Program &ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected one assignment statement to survive.";
            if (ast.execution_steps.empty()) return nullptr;

            const auto *assign = dynamic_cast<const Assignment *>(ast.execution_steps.front().get());
            EXPECT_NE(assign, nullptr) << "Expected statement to be an Assignment.";
            if (!assign) return nullptr;

            const auto *cond = dynamic_cast<const ConditionalExpression *>(assign->value.get());
            EXPECT_NE(cond, nullptr) << "Expected assigned value to be a ConditionalExpression.";
            return cond;
        }

        auto ExpectConditionalBranches(bool has_cond, bool has_then, bool has_else) {
            return [has_cond, has_then, has_else](const Program &ast) {
                const auto *cond = GetAssignedConditional(ast);
                ASSERT_NE(cond, nullptr);

                if (has_cond) {
                    EXPECT_NE(cond->condition, nullptr) << "Expected condition to be present.";
                } else {
                    EXPECT_EQ(cond->condition, nullptr) << "Expected condition to be absent/nulled.";
                }

                if (has_then) {
                    EXPECT_NE(cond->then_branch, nullptr) << "Expected then_branch to be present.";
                } else {
                    EXPECT_EQ(cond->then_branch, nullptr) << "Expected then_branch to be absent/nulled.";
                }

                if (has_else) {
                    EXPECT_NE(cond->else_branch, nullptr) << "Expected else_branch to be present.";
                } else {
                    EXPECT_EQ(cond->else_branch, nullptr) << "Expected else_branch to be absent/nulled.";
                }
            };
        }

        auto ExpectNoExecutionSteps() {
            return [](const Program &ast) {
                ASSERT_EQ(ast.execution_steps.size(), 0) << "Expected statement to be discarded entirely.";
            };
        }
    }

    class ConditionalParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(ConditionalParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserExhaustiveStressTests,
        ConditionalParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "missing_then_token_recovers_branches",
            "let a = if x 1 else 2\n",
            {{Err::MissingThenToken, 1, 14}},
            ExpectConditionalBranches(true, true, true)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_else_token_recovers_branches",
            "let a = if x then 1 2\n",
            {{Err::MissingElseToken, 1, 21}},
            ExpectConditionalBranches(true, true, true)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_both_then_and_else_tokens",
            "let a = if x 1 2\n",
            {
            {Err::MissingThenToken, 1, 14},
            {Err::MissingElseToken, 1, 16}
            },
            ExpectConditionalBranches(true, true, true)
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_in_condition_recovers_then_and_else",
            "let a = if x + * then 1 else 2\n",
            {{Err::InvalidExpression, 1, 16}},
            ExpectConditionalBranches(false, true, true)
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_in_then_branch_recovers_else",
            "let a = if x then 1 + * else 2\n",
            {{Err::InvalidExpression, 1, 23}},
            ExpectConditionalBranches(true, false, true)
            },
            ParserErrorsSynchronizationTestCase{
            "empty_condition_recovers_then_and_else",
            "let a = if then 1 else 2\n",
            {{Err::InvalidExpression, 1, 12}},
            ExpectConditionalBranches(false, true, true)
            },
            ParserErrorsSynchronizationTestCase{
            "empty_then_branch_recovers_else",
            "let a = if x then else 2\n",
            {{Err::InvalidExpression, 1, 19}},
            ExpectConditionalBranches(true, false, true)
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_in_condition_and_then_branch_recovers_else",
            "let a = if * then * else 2\n",
            {
            {Err::InvalidExpression, 1, 12},
            {Err::InvalidExpression, 1, 19}
            },
            ExpectConditionalBranches(false, false, true)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_expression_after_else_bubbles_up_and_discards_statement",
            "let a = if x then 1 else +\n",
            {{Err::InvalidExpression, 1, 27}},
            ExpectNoExecutionSteps()
            },
            ParserErrorsSynchronizationTestCase{
            "grouping_synchronizes_internal_errors_in_condition",
            "let a = if (x + *) then 1 else 2\n",
            {{Err::InvalidExpression, 1, 17}},
            [](const Program& ast) {
            const auto* cond = GetAssignedConditional(ast);
            ASSERT_NE(cond, nullptr);
            EXPECT_NE(cond->then_branch, nullptr);
            EXPECT_NE(cond->else_branch, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_conditional_error_in_inner_condition_recovers_outer_and_inner",
            "let a = if (if * then 1 else 2) then 3 else 4\n",
            {{Err::InvalidExpression, 1, 16}},
            [](const Program& ast) {
            const auto* outer = GetAssignedConditional(ast);
            ASSERT_NE(outer, nullptr);
            EXPECT_NE(outer->then_branch, nullptr);
            EXPECT_NE(outer->else_branch, nullptr);

            auto* grouping = dynamic_cast<const GroupingExpression*>(outer->condition.get());
            ASSERT_NE(grouping, nullptr);
            auto* inner = dynamic_cast<const ConditionalExpression*>(grouping->expression.get());
            ASSERT_NE(inner, nullptr);

            EXPECT_EQ(inner->condition, nullptr) << "Inner condition should be null due to syntax error";
            EXPECT_NE(inner->then_branch, nullptr);
            EXPECT_NE(inner->else_branch, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_conditional_error_in_inner_then_recovers_outer",
            "let a = if x then (if y then * else 2) else 3\n",
            {{Err::InvalidExpression, 1, 30}},
            [](const Program& ast) {
            const auto* outer = GetAssignedConditional(ast);
            ASSERT_NE(outer, nullptr);
            EXPECT_NE(outer->condition, nullptr);

            auto* grouping = dynamic_cast<const GroupingExpression*>(outer->then_branch.get());
            ASSERT_NE(grouping, nullptr);
            auto* inner = dynamic_cast<const ConditionalExpression*>(grouping->expression.get());
            ASSERT_NE(inner, nullptr);

            EXPECT_NE(inner->condition, nullptr);
            EXPECT_EQ(inner->then_branch, nullptr) << "Inner then branch should be null due to syntax error";
            EXPECT_NE(inner->else_branch, nullptr);

            EXPECT_NE(outer->else_branch, nullptr) << "Outer else branch should survive perfectly";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_conditional_error_in_inner_else_bubbles_and_recovers_outer_else",
            "let a = if x then (if y then 1 else *) else 3\n",
            {{Err::InvalidExpression, 1, 37}},
            [](const Program& ast) {
            const auto* outer = GetAssignedConditional(ast);
            ASSERT_NE(outer, nullptr);
            EXPECT_NE(outer->condition, nullptr);

            EXPECT_NE(outer->then_branch, nullptr);

            EXPECT_NE(outer->else_branch, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unterminated_grouping_in_condition_recovers_then_and_else",
            "let a = if (x + 1 then 2 else 3\n",
            {{Err::ExpectedRightParenAfterExpression, 1, 19}},
            [](const Program& ast) {
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
            "missing_else_with_subsequent_statement_discards_assignment_and_recovers_next",
            "let a = if x then 1\n"
            "let b = 2\n",
            {
            {Err::MissingElseToken, 1, 20},
            {Err::InvalidExpression, 1, 20}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            const auto* assign = dynamic_cast<const Assignment*>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "correct_ast_structure_for_complex_valid_conditional",
            "let a = if x > 5 then y * 2 else z - 3\n",
            {},
            [](const Program& ast) {
            const auto* cond = GetAssignedConditional(ast);
            ASSERT_NE(cond, nullptr);

            auto* condition = dynamic_cast<const BinaryExpression*>(cond->condition.get());
            ASSERT_NE(condition, nullptr);
            EXPECT_EQ(condition->op, TokenType::Greater);

            auto* then_branch = dynamic_cast<const BinaryExpression*>(cond->then_branch.get());
            ASSERT_NE(then_branch, nullptr);
            EXPECT_EQ(then_branch->op, TokenType::Star);

            auto* else_branch = dynamic_cast<const BinaryExpression*>(cond->else_branch.get());
            ASSERT_NE(else_branch, nullptr);
            EXPECT_EQ(else_branch->op, TokenType::Minus);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_then_and_else_tokens_but_valid_expressions_maintains_ast_integrity",
            "let a = if x > 5 y * 2 z - 3\n",
            {
            {Err::MissingThenToken, 1, 18},
            {Err::MissingElseToken, 1, 24}
            },
            [](const Program& ast) {
            const auto* cond = GetAssignedConditional(ast);
            ASSERT_NE(cond, nullptr);

            auto* condition = dynamic_cast<const BinaryExpression*>(cond->condition.get());
            ASSERT_NE(condition, nullptr);
            EXPECT_EQ(condition->op, TokenType::Greater);

            auto* then_branch = dynamic_cast<const BinaryExpression*>(cond->then_branch.get());
            ASSERT_NE(then_branch, nullptr);
            EXPECT_EQ(then_branch->op, TokenType::Star);

            auto* else_branch = dynamic_cast<const BinaryExpression*>(cond->else_branch.get());
            ASSERT_NE(else_branch, nullptr);
            EXPECT_EQ(else_branch->op, TokenType::Minus);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unterminated_grouping_in_then_branch_recovers_else",
            "let a = if x then (1 + 2 else 3\n",
            {{Err::ExpectedRightParenAfterExpression, 1, 26}},
            [](const Program& ast) {
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
            "unterminated_grouping_in_else_branch_bubbles_up_and_recovers_next_statement",
            "let a = if x then 1 else (2 + 3\n"
            "let b = 2\n",
            {
            {Err::ExpectedRightParenAfterExpression, 1, 32},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            const auto* assign = dynamic_cast<const Assignment*>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "a");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_operator_in_condition_recovers_then_and_else",
            "let a = if x + then 1 else 2\n",
            {{Err::InvalidExpression, 1, 16}},
            [](const Program& ast) {
            const auto* cond = GetAssignedConditional(ast);
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->condition, nullptr);
            EXPECT_NE(cond->then_branch, nullptr);
            EXPECT_NE(cond->else_branch, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_operator_in_then_branch_recovers_else",
            "let a = if x then 1 + else 2\n",
            {{Err::InvalidExpression, 1, 23}},
            [](const Program& ast) {
            const auto* cond = GetAssignedConditional(ast);
            ASSERT_NE(cond, nullptr);
            EXPECT_NE(cond->condition, nullptr);
            EXPECT_EQ(cond->then_branch, nullptr);
            EXPECT_NE(cond->else_branch, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bare_if_token_at_eof_discards_gracefully",
            "let a = if\n",
            {
            {Err::InvalidExpression, 1, 11},
            {Err::MissingThenToken, 1, 11},
            {Err::InvalidExpression, 1, 11},
            {Err::MissingElseToken, 1, 11},
            {Err::InvalidExpression, 1, 11}
            },
            ExpectNoExecutionSteps()
            },
            ParserErrorsSynchronizationTestCase{
            "nested_if_as_condition_missing_parenthesis_recovers_outer_branches",
            "let a = if (if x then 1 else 2 then 3 else 4\n",
            {{Err::ExpectedRightParenAfterExpression, 1, 32}},
            [](const Program& ast) {
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
            },
            ParserErrorsSynchronizationTestCase{
            "chained_else_if_error_in_middle_condition_preserves_chain",
            "let a = if x then 1 else if y + then 2 else 3\n",
            {{Err::InvalidExpression, 1, 33}},
            [](const Program& ast) {
            const auto* cond = GetAssignedConditional(ast);
            ASSERT_NE(cond, nullptr);

            EXPECT_NE(cond->condition, nullptr);
            EXPECT_NE(cond->then_branch, nullptr);

            auto* inner_cond = dynamic_cast<const ConditionalExpression*>(cond->else_branch.get());
            ASSERT_NE(inner_cond, nullptr);

            EXPECT_EQ(inner_cond->condition, nullptr) << "Inner condition should be null due to dangling '+'";
            EXPECT_NE(inner_cond->then_branch, nullptr) << "Inner then branch should recover";
            EXPECT_NE(inner_cond->else_branch, nullptr) << "Inner else branch should recover";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "conditional_inside_binary_expression_preserves_outer_math",
            "let a = 10 * (if x + then 1 else 2) + 5\n",
            {{Err::InvalidExpression, 1, 22}},
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            const auto* assign = dynamic_cast<const Assignment*>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr);

            auto* plus_expr = dynamic_cast<const BinaryExpression*>(assign->value.get());
            ASSERT_NE(plus_expr, nullptr) << "Outer addition was destroyed!";
            EXPECT_EQ(plus_expr->op, TokenType::Plus);

            auto* star_expr = dynamic_cast<const BinaryExpression*>(plus_expr->left.get());
            ASSERT_NE(star_expr, nullptr) << "Outer multiplication was destroyed!";
            EXPECT_EQ(star_expr->op, TokenType::Star);

            auto* grouping = dynamic_cast<const GroupingExpression*>(star_expr->right.get());
            ASSERT_NE(grouping, nullptr);

            auto* cond = dynamic_cast<const ConditionalExpression*>(grouping->expression.get());
            ASSERT_NE(cond, nullptr) << "Inner conditional was destroyed!";

            EXPECT_EQ(cond->condition, nullptr) << "Condition should be null";
            EXPECT_NE(cond->then_branch, nullptr) << "Then branch should be intact";
            EXPECT_NE(cond->else_branch, nullptr) << "Else branch should be intact";
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
