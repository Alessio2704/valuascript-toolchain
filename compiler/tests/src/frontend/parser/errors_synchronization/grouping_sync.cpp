#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        void ExpectGroupingLiteral(const Program &ast, std::function<void(Expression *)> inner_checker = nullptr) {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto *grouping = dynamic_cast<GroupingExpression *>(assign->value.get());
            ASSERT_NE(grouping, nullptr) << "Assignment value is not a GroupingExpression.";

            if (inner_checker && grouping->expression) {
                inner_checker(grouping->expression.get());
            }
        }

        auto ExpectGrouping() {
            return [](const Program &ast) {
                ExpectGroupingLiteral(ast);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

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
                .test_name = "grouping_unclosed",
                .source_code = "let a = ( 1 + 2 \nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 15} },
                .verify_ast = [](const Program& ast) {
                    ExpectGroupingLiteral(ast);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_closed_with_wrong_bracket",
                .source_code = "let a = ( 1 + 2 ]\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 15} },
                .verify_ast = ExpectGrouping()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_mismatched_bracket_in_nested_list",
                .source_code = "let a = [ ( 1 + 2 ], 3 ]\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 17} },
                .verify_ast = [](const Program& ast) {
                    auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
                    auto *tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
                    ASSERT_NE(tensor, nullptr);
                    EXPECT_EQ(tensor->elements.size(), 2);
                    EXPECT_NE(dynamic_cast<GroupingExpression*>(tensor->elements[0].get()), nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_with_top_level_declaration",
                .source_code = "let a = ( let x = 1 )\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::TopLevelDeclarationNotAllowedHere, .line = 1, .column = 11},
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_eof_mid_expression",
                .source_code = "let a = ( 1 + ",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 1, .column = 14}, {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 13} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_leading_comma_error",
                .source_code = "let a = ( , 1 )\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 1, .column = 11} },
                .verify_ast = [](const Program& ast) {
                    auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
                    ASSERT_NE(dynamic_cast<TupleLiteral*>(assign->value.get()), nullptr);
                }
            },
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
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_interrupted_by_directive",
                .source_code = "let a = ( 1 + #hidden )\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::TopLevelDeclarationNotAllowedHere, .line = 1, .column = 15},
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                    EXPECT_EQ(ast.directives.size(), 0);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_mismatched_closer_no_comma_heuristic",
                .source_code = "let a = ( 1 + 2 }\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 15} },
                .verify_ast = ExpectGrouping()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "grouping_with_reserved_keyword_as_identifier",
                .source_code = "let a = ( let )\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 11} },
                .verify_ast = [](const Program& ast) {
                    auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto* group = dynamic_cast<GroupingExpression*>(assign->value.get());
                    ASSERT_NE(dynamic_cast<IdentifierAccess*>(group->expression.get()), nullptr);
                }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}
