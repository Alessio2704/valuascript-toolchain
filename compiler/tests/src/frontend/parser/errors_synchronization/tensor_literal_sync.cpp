#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "frontend/parser/helpers/node_matchers.h"

namespace valuascript::compiler::test
{
    namespace
    {
        void ExpectTensorElements(const Program& ast, const std::vector<std::string>& expected_numbers)
        {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto* tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            ASSERT_NE(tensor, nullptr) << "Assignment value is not a TensorLiteral.";

            ASSERT_EQ(tensor->elements.size(), expected_numbers.size()) << "Tensor element count mismatch!";

            for (size_t i = 0; i < expected_numbers.size(); ++i)
            {
                if (!expected_numbers[i].empty())
                {
                    auto* num = dynamic_cast<NumberLiteral*>(tensor->elements[i].get());
                    ASSERT_NE(num, nullptr) << "Expected number literal at index " << i;
                    EXPECT_EQ(num->value, expected_numbers[i]) << "Value mismatch at index " << i;
                }
            }
        }

        auto ExpectTensor(std::vector<std::string> numbers)
        {
            return [n = std::move(numbers)](const Program& ast)
            {
                ExpectTensorElements(ast, n);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

    class TensorLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(TensorLiteralParserSynchronizationTest, SynchronizesTensorLiteralErrors)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TensorLiteralStressTests,
        TensorLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_missing_closing_bracket",
                .source_code = "let a =[ 1, 2 \nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 13} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_closed_with_wrong_brace",
                .source_code = "let a =[ 1, 2 }\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 13}
                },
                .verify_ast = ExpectTensor({"1", "2"})
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_missing_bracket_before_next_stmt",
                .source_code = "let a = [ 1, 2\nlet b = 3\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 14} },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 3);
                    auto* b_assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(b_assign, nullptr);
                    EXPECT_EQ(b_assign->targets[0].name, "a");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_eof_immediately_after_bracket",
                .source_code = "let a = [",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 9} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_with_mismatched_nested_closer",
                .source_code = "let a = [ ( 1 + 2 ], 3 ]\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 17} },
                .verify_ast = [](const Program& ast) {
                    const auto assignment = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto tensor = dynamic_cast<TensorLiteral*>(assignment->value.get());
                    ASSERT_EQ(tensor->elements.size(), 2);
                    auto first_elem = dynamic_cast<BinaryExpression*>(unwrap_grouping(tensor->elements[0].get()));
                    ASSERT_NE(first_elem, nullptr);
                    ASSERT_EQ(first_elem->op, TokenType::Plus);
                    auto second_elem = dynamic_cast<NumberLiteral*>(tensor->elements[1].get());
                    ASSERT_NE(second_elem, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_with_unclosed_string_element",
                .source_code = "let a = [ \"unclosed, 2 ]\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = LexerErrorCode::UnclosedString, .line = 1, .column = 11},
                    {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 24}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
                    EXPECT_EQ(tensor->elements.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_with_illegal_statement_inside_new_line",
                .source_code = "let a = [ \nfunc f() -> void {}, \n1 \n]\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 9},
                    {.code = Err::InvalidExpression, .line = 2, .column = 20},
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.execution_steps.size(), 2);

                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
                    EXPECT_EQ(tensor->elements.size(), 0);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_multiline_deeply_nested_failures",
                .source_code = "let a =[\n  (1 + *),\n  { key: * }\n]\nlet b = [\n[ *, 2 ],\n  f(a: *)\n]\nlet recovery = 100\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 2, .column = 8},
                    {.code = Err::InvalidExpression, .line = 3, .column = 10},
                    {.code = Err::InvalidExpression, .line = 6, .column = 3},
                    {.code = Err::InvalidExpression, .line = 7, .column = 8}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 3) << "AST must have exactly 3 statements.";

                    auto* assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto* tensor_a = dynamic_cast<TensorLiteral*>(assign_a->value.get());
                    ASSERT_NE(tensor_a, nullptr);
                    EXPECT_EQ(tensor_a->elements.size(), 2);
                    auto tensor_a_grouping = dynamic_cast<GroupingExpression*>(tensor_a->elements[0].get());
                    ASSERT_NE(tensor_a_grouping, nullptr);
                    ASSERT_NE(tensor_a_grouping->expression, nullptr);
                    auto tensor_a_dict = dynamic_cast<DictLiteral*>(tensor_a->elements[1].get());
                    EXPECT_EQ(tensor_a_dict->elements.size(), 1);
                    EXPECT_EQ(tensor_a_dict->elements[0].value.get(), nullptr); auto* assign_b = dynamic_cast<Assignment*>(ast.
                        execution_steps[1].get());
                    auto* tensor_b = dynamic_cast<TensorLiteral*>(assign_b->value.get());
                    ASSERT_NE(tensor_b, nullptr);
                    EXPECT_EQ(tensor_b->elements.size(), 2);

                    auto* inner_tensor = dynamic_cast<TensorLiteral*>(tensor_b->elements[0].get());
                    ASSERT_NE(inner_tensor, nullptr) << "First element of b should be a TensorLiteral";
                    EXPECT_EQ(inner_tensor->elements.size(), 2);

                    auto* assign_rec = dynamic_cast<Assignment*>(ast.execution_steps[2].get());
                    ASSERT_NE(assign_rec, nullptr);
                    EXPECT_EQ(assign_rec->targets[0].name, "recovery");
                    ASSERT_NE(dynamic_cast<NumberLiteral*>(assign_rec->value.get()), nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_multiline_dangling_operators_hitting_newline",
                .source_code = "let a =[\n  10 +\n]\nlet b =[\n  20,\n  30 ==\n]\nlet recovery = [ 40 ]\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 2, .column = 6},
                    {.code = Err::InvalidExpression, .line = 6, .column = 6}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 3) << "AST must have exactly 3 statements.";

                    auto* assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto* tensor_a = dynamic_cast<TensorLiteral*>(assign_a->value.get());
                    EXPECT_EQ(tensor_a->elements.size(), 1);

                    auto* assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
                    auto* tensor_b = dynamic_cast<TensorLiteral*>(assign_b->value.get());
                    EXPECT_EQ(tensor_b->elements.size(), 2);

                    auto* assign_rec = dynamic_cast<Assignment*>(ast.execution_steps[2].get());
                    auto* tensor_rec = dynamic_cast<TensorLiteral*>(assign_rec->value.get());
                    EXPECT_EQ(tensor_rec->elements.size(), 1);
                }
            }
        ),
        TestNameGenerator{}
    );
}
