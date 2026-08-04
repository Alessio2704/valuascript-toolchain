#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        void ExpectTupleLiteral(const Program &ast, size_t expected_size) {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto *tuple_node = dynamic_cast<TupleLiteral *>(assign->value.get());
            ASSERT_NE(tuple_node, nullptr) << "Assignment value is not a TupleLiteral.";

            EXPECT_EQ(tuple_node->elements.size(), expected_size) << "Tuple size mismatch!";
        }

        auto ExpectTuple(size_t expected_size) {
            return [expected_size](const Program &ast) {
                ExpectTupleLiteral(ast, expected_size);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

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
                .test_name = "tuple_missing_closing_paren",
                .source_code = "let a = (1, 2 \nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 13} },
                .verify_ast = ExpectTuple(2)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_missing_multiple_closing_paren",
                .source_code = "let a = (1, 2 \nlet b = (3, 4 \nlet c = (5, 6 \nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 13},
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 2, .column = 13},
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 3, .column = 13}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 4);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "array_inside_tuple_first_element_error",
                .source_code = "let a = ([1, 2)\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 14} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                    auto assignment = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto grouping = dynamic_cast<GroupingExpression*>(assignment->value.get());
                    EXPECT_NE(grouping, nullptr);
                    EXPECT_NE(grouping->expression, nullptr);
                    auto tensor = dynamic_cast<TensorLiteral*>(grouping->expression.get());
                    EXPECT_EQ(tensor->elements.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "array_inside_tuple_second_element_error",
                .source_code = "let a = (1, [1, 2)\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 17} },
                .verify_ast = ExpectTuple(2)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_total_mangle",
                .source_code = "let a = (1, let y = 2\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 1, .column = 13},
                    {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 21}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                    const auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    const auto assign_1_val = dynamic_cast<TupleLiteral*>(assign_1->value.get());
                    ASSERT_NE(assign_1_val, nullptr);
                    ASSERT_EQ(assign_1_val->elements.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_with_top_level_declaration_inside",
                .source_code = "let a = (1, let b = 2, 3)\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 1, .column = 13}
                },
                .verify_ast = ExpectTuple(3)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_eof_after_comma",
                .source_code = "let a = (1, \n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 11} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                    const auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    const auto assign_1_val = dynamic_cast<TupleLiteral*>(assign_1->value.get());
                    ASSERT_NE(assign_1_val, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_eof_after_first_element",
                .source_code = "let a = (1 \n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 10} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                    const auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    const auto assign_1_val = dynamic_cast<GroupingExpression*>(assign_1->value.get());
                    ASSERT_NE(assign_1_val, nullptr);
                    ASSERT_NE(assign_1_val->expression, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_closed_with_wrong_bracket",
                .source_code = "let a = (1, 2]\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterTupleElements, .line = 1, .column = 13} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_with_mixed_closing_brackets_inside",
                .source_code = "let a = (1,[2, 3}, 4)\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 16}
                },
                .verify_ast = ExpectTuple(3)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_too_many_closing_parens",
                .source_code = "let a = (1, 2))\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 1, .column = 15}
                },
                .verify_ast = ExpectTuple(2)
            },
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
