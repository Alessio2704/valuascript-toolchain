#include <gtest/gtest.h>
#include "../helpers/parser_errors_synchronization_base.h"
#include "frontend/parser/ast.h"

namespace valuascript::compiler::test {
    namespace {
        void ExpectIdentifier(const Expression *expr, const std::string &name) {
            auto id = dynamic_cast<const IdentifierAccess *>(expr);
            ASSERT_NE(id, nullptr) << "Expected IdentifierAccess, but got " << (
                                      expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(id->name, name);
        }

        void ExpectNumber(const Expression *expr, const std::string &value) {
            auto num = dynamic_cast<const NumberLiteral *>(expr);
            ASSERT_NE(num, nullptr) << "Expected NumberLiteral, but got " << (expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(num->value, value);
        }

        void ExpectBinary(const Expression *expr, TokenType expected_op,
                          const std::function<void(const Expression *)>& left_verifier,
                          const std::function<void(const Expression *)>& right_verifier) {
            auto bin = dynamic_cast<const BinaryExpression *>(expr);
            ASSERT_NE(bin, nullptr) << "Expected BinaryExpression, but got " << (
                                       expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(bin->op, expected_op);

            if (left_verifier) left_verifier(bin->left.get());
            else
                EXPECT_EQ(bin->left, nullptr);

            if (right_verifier) right_verifier(bin->right.get());
            else
                EXPECT_EQ(bin->right, nullptr);
        }

        void ExpectGrouping(const Expression *expr, const std::function<void(const Expression *)>& inner_verifier) {
            auto grp = dynamic_cast<const GroupingExpression *>(expr);
            ASSERT_NE(grp, nullptr) << "Expected GroupingExpression, but got " << (expr
                                           ? typeid(*expr).name()
                                           : "nullptr");

            if (inner_verifier) inner_verifier(grp->expression.get());
            else
                EXPECT_EQ(grp->expression, nullptr);
        }

        auto VerifyAssignmentValue(const std::function<void(const Expression *)>& value_verifier, size_t expected_steps = 1,
                                   size_t step_index = 0) {
            return [=](const Program &ast) {
                ASSERT_EQ(ast.execution_steps.size(), expected_steps) << "Execution step count mismatch";
                auto assign = dynamic_cast<Assignment *>(ast.execution_steps[step_index].get());
                ASSERT_NE(assign, nullptr) << "Expected step " << step_index << " to be an Assignment";
                value_verifier(assign->value.get());
            };
        }
    }

    class BinaryAndUnaryErrorsParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(BinaryAndUnaryErrorsParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        BinaryAndUnaryErrorsStressTests,
        BinaryAndUnaryErrorsParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "binary_missing_right_eof",
            "let a = 1 +",
            { {Err::InvalidExpression, 1, 12} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 0) << "Statement should be dropped";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "binary_missing_right_newline",
            "let a = 1 +\n"
            "let b = 2\n",
            { {Err::InvalidExpression, 1, 11} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "binary_missing_left",
            "let a = * 2\nlet b = 2\n",
            { {Err::InvalidExpression, 1, 9} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "binary_invalid_right",
            "let a = 1 + * 2\nlet b = 2\n",
            { {Err::InvalidExpression, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unary_missing_operand",
            "let a = -\n"
            "let b = 2\n",
            { {Err::InvalidExpression, 1, 9} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unary_invalid_operand",
            "let a = - *\nlet b = 2\n",
            { {Err::InvalidExpression, 1, 11} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_unary_invalid_operand",
            "let a = - *\n",
            { {Err::InvalidExpression, 1, 11} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "binary_chained_comparisons",
            "let a = 1 < 2 < 3\nlet b = 2\n",
            { {Err::ChainingNotAllowedForComparisonOperations, 1, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "grouping_missing_operator",
            "let a = (1 2)\n",
            { {Err::MissingOperatorInsideGrouping, 1, 12} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, [](auto inner) { ExpectNumber(inner, "1"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_recovers_binary_error_and_returns_partial_ast",
            "let a = [1 +, 2]\n",
            { {Err::InvalidExpression, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                auto tensor = dynamic_cast<const TensorLiteral*>(expr);
                ASSERT_NE(tensor, nullptr);
                ASSERT_EQ(tensor->elements.size(), 1);
                ExpectNumber(tensor->elements[0].get(), "2");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "function_call_recovers_binary_error_in_argument",
            "let a = test(arg1: 1 +, arg2: 2)\n",
            { {Err::InvalidExpression, 1, 23} },
            VerifyAssignmentValue([](auto expr) {
                auto call = dynamic_cast<const FunctionCall*>(expr);
                ASSERT_NE(call, nullptr);
                ExpectIdentifier(call->target.get(), "test");
                ASSERT_EQ(call->arguments.size(), 2);
                EXPECT_EQ(call->arguments[0].first, "arg1");
                EXPECT_EQ(call->arguments[1].first, "arg2");
                ExpectNumber(call->arguments[1].second.get(), "2");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "binary_expr_survives_with_recovered_tensor_operands",
            "let a = [*, 1] + [2, *]\n",
            {
            {Err::InvalidExpression, 1, 10},
            {Err::InvalidExpression, 1, 22}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBinary(expr, TokenType::Plus,
                    [](auto left) {
                    auto tensor_left = dynamic_cast<const TensorLiteral*>(left);
                    ASSERT_NE(tensor_left, nullptr);
                    ASSERT_EQ(tensor_left->elements.size(), 1);
                    ExpectNumber(tensor_left->elements[0].get(), "1");
                    },
                    [](auto right) {
                    auto tensor_right = dynamic_cast<const TensorLiteral*>(right);
                    ASSERT_NE(tensor_right, nullptr);
                    ASSERT_EQ(tensor_right->elements.size(), 1);
                    ExpectNumber(tensor_right->elements[0].get(), "2");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_standalone_statement",
            "1 + 1\nlet b = 2\n",
            { {Err::InvalidStandaloneStatement, 1, 5} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "chained_comparisons_inside_grouping",
            "let a = (1 < 2 < 3)\n",
            { {Err::ChainingNotAllowedForComparisonOperations, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "unary_inside_failed_grouping",
            "let a = (-)\n",
            { {Err::InvalidExpression, 1, 11} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "chained_comparison_in_tensor_recovers",
            "let a = [1 < 2 < 3, 4]\n",
            { {Err::ChainingNotAllowedForComparisonOperations, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                auto tensor = dynamic_cast<const TensorLiteral*>(expr);
                ASSERT_NE(tensor, nullptr);
                ASSERT_EQ(tensor->elements.size(), 1);
                ExpectNumber(tensor->elements[0].get(), "4");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "binary_multiple_operators_in_row",
            "let a = 1 + - * 2\nlet b = 1\n",
            { {Err::InvalidExpression, 1, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unary_chain_with_invalid_operator",
            "let a = - + * 2\nlet b = 2\n",
            { {Err::InvalidExpression, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unary_not_missing_operand",
            "let a = not\n"
            "let b = 2\n",
            { {Err::InvalidExpression, 1, 9} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "binary_before_closing_grouping",
            "let a = (1 + )\n",
            { {Err::InvalidExpression, 1, 14} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "chained_comparisons_mixed_operators",
            "let a = 1 < 2 >= 3\nlet b = 2\n",
            { {Err::ChainingNotAllowedForComparisonOperations, 1, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "chained_equality_operators",
            "let a = 1 == 2 != 3\n",
            { {Err::ChainingNotAllowedForComparisonOperations, 1, 16} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "right_associative_power_missing_operand",
            "let a = 2 ^ ^ 3\n",
            { {Err::InvalidExpression, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "binary_operator_dangling_on_next_line",
            "let a = 1\n"
            "* 2\n"
            "let b = 3\n",
            { {Err::InvalidExpression, 2, 1} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "deep_precedence_error_in_list_recovers",
            "let a = [1 + 2 * *, 3 - 4]\n",
            { {Err::InvalidExpression, 1, 18} },
            VerifyAssignmentValue([](auto expr) {
                auto tensor = dynamic_cast<const TensorLiteral*>(expr);
                ASSERT_NE(tensor, nullptr);
                ASSERT_EQ(tensor->elements.size(), 1);
                ExpectBinary(tensor->elements[0].get(), TokenType::Minus,
                    [](auto left) { ExpectNumber(left, "3"); },
                    [](auto right) { ExpectNumber(right, "4"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "complex_tuple_partial_ast_recovery",
            "let a = (1 + 2, 3 *, 4 - 1)\n",
            { {Err::InvalidExpression, 1, 20} },
            VerifyAssignmentValue([](auto expr) {
                auto tuple = dynamic_cast<const TupleLiteral*>(expr);
                ASSERT_NE(tuple, nullptr);
                ASSERT_EQ(tuple->elements.size(), 2);
                ExpectBinary(tuple->elements[0].get(), TokenType::Plus,
                    [](auto left) { ExpectNumber(left, "1"); },
                    [](auto right) { ExpectNumber(right, "2"); });
                ExpectBinary(tuple->elements[1].get(), TokenType::Minus,
                    [](auto left) { ExpectNumber(left, "4"); },
                    [](auto right) { ExpectNumber(right, "1"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "binary_missing_rhs_in_dictionary_value",
            "let a = {key1: 1 +, key2: 2}\n",
            { {Err::InvalidExpression, 1, 19} },
            VerifyAssignmentValue([](auto expr) {
                auto dict = dynamic_cast<const DictLiteral*>(expr);
                ASSERT_NE(dict, nullptr);
                ASSERT_EQ(dict->elements.size(), 2);
                EXPECT_EQ(dict->elements[0].key, "key1");
                EXPECT_EQ(dict->elements[0].value.get(), nullptr);
                EXPECT_EQ(dict->elements[1].key, "key2");
                ExpectNumber(dict->elements[1].value.get(), "2");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "unary_inside_function_call_argument",
            "let a = test(arg1: -, arg2: 42)\n",
            { {Err::InvalidExpression, 1, 21} },
            VerifyAssignmentValue([](auto expr) {
                auto call = dynamic_cast<const FunctionCall*>(expr);
                ASSERT_NE(call, nullptr);
                ASSERT_EQ(call->arguments.size(), 2);
                EXPECT_EQ(call->arguments[0].first, "arg1");
                EXPECT_EQ(call->arguments[1].first, "arg2");
                ExpectNumber(call->arguments[1].second.get(), "42");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "grouping_containing_only_operator",
            "let a = (+)\n",
            { {Err::InvalidExpression, 1, 11} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "nested_parentheses_with_binary_error",
            "let a = ((1 + * 2))\n",
            { {Err::InvalidExpression, 1, 15} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, [](auto inner1) {
                    ExpectGrouping(inner1, nullptr);
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "unary_not_applied_to_binary_missing_rhs",
            "let a = not(1 +)\n",
            { {Err::InvalidExpression, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                auto unary = dynamic_cast<const UnaryExpression*>(expr);
                ASSERT_NE(unary, nullptr);
                EXPECT_EQ(unary->op, TokenType::Not);
                ExpectGrouping(unary->right.get(), nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_rejected_at_top_level",
            "let a = 1\n"
            "* 2\n",
            { {Err::InvalidExpression, 2, 1} },
            VerifyAssignmentValue([](auto expr) {
                ExpectNumber(expr, "1");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_unary_becomes_standalone_and_is_rejected",
            "let a = 1\n"
            "+ 2\n",
            { {Err::InvalidStandaloneStatement, 2, 3} },
            VerifyAssignmentValue([](auto expr) {
                ExpectNumber(expr, "1");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_operator_at_end_of_line_is_allowed",
            "let a = 1 +\n"
            "2\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                ExpectBinary(expr, TokenType::Plus,
                    [](auto left) { ExpectNumber(left, "1"); },
                    [](auto right) { ExpectNumber(right, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_allowed_inside_parentheses",
            "let a = (1\n"
            "+ 2)\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, [](auto inner) {
                    ExpectBinary(inner, TokenType::Plus,
                        [](auto left) { ExpectNumber(left, "1"); },
                        [](auto right) { ExpectNumber(right, "2"); });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_allowed_inside_tensor_literal",
            "let a = [1\n"
            "* 2, 3]\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                auto tensor = dynamic_cast<const TensorLiteral*>(expr);
                ASSERT_NE(tensor, nullptr);
                ASSERT_EQ(tensor->elements.size(), 2);
                ExpectBinary(tensor->elements[0].get(), TokenType::Star,
                    [](auto left) { ExpectNumber(left, "1"); },
                    [](auto right) { ExpectNumber(right, "2"); });
                ExpectNumber(tensor->elements[1].get(), "3");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_with_trailing_operator_inside_dictionary",
            "let a = {key: 1 -\n"
            "2}\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                auto dict = dynamic_cast<const DictLiteral*>(expr);
                ASSERT_NE(dict, nullptr);
                ASSERT_EQ(dict->elements.size(), 1);
                EXPECT_EQ(dict->elements[0].key, "key");
                ExpectBinary(dict->elements[0].value.get(), TokenType::Minus,
                    [](auto left) { ExpectNumber(left, "1"); },
                    [](auto right) { ExpectNumber(right, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_wrapped_in_parens_inside_dictionary",
            "let a = {key: (1\n"
            "- 2)}\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                auto dict = dynamic_cast<const DictLiteral*>(expr);
                ASSERT_NE(dict, nullptr);
                ASSERT_EQ(dict->elements.size(), 1);
                EXPECT_EQ(dict->elements[0].key, "key");
                ExpectGrouping(dict->elements[0].value.get(), [](auto inner) {
                    ExpectBinary(inner, TokenType::Minus,
                        [](auto left) { ExpectNumber(left, "1"); },
                        [](auto right) { ExpectNumber(right, "2"); });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_rejected_inside_dictionary_literal",
            "let a = {key: 1\n"
            "-2}\n",
            { {Err::MissingOperator, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                auto dict = dynamic_cast<const DictLiteral*>(expr);
                ASSERT_NE(dict, nullptr);
                ASSERT_EQ(dict->elements.size(), 1);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_not_unary_rejected_inside_dictionary_literal",
            "let a = {key: 1\n"
            "*2}\n",
            { {Err::MissingOperator, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                auto dict = dynamic_cast<const DictLiteral*>(expr);
                ASSERT_NE(dict, nullptr);
                ASSERT_EQ(dict->elements.size(), 1);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_allowed_inside_function_call_arguments",
            "let a = test(arg: 1\n"
            "/ 2)\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                auto call = dynamic_cast<const FunctionCall*>(expr);
                ASSERT_NE(call, nullptr);
                ASSERT_EQ(call->arguments.size(), 1);
                ExpectBinary(call->arguments[0].second.get(), TokenType::Slash,
                    [](auto left) { ExpectNumber(left, "1"); },
                    [](auto right) { ExpectNumber(right, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "complex_multiline_mixed_groupings",
            "let a = [\n"
            "  (1\n"
            "  + 2)\n"
            "  * 3\n"
            "]\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                auto tensor = dynamic_cast<const TensorLiteral*>(expr);
                ASSERT_NE(tensor, nullptr);
                ASSERT_EQ(tensor->elements.size(), 1);
                ExpectBinary(tensor->elements[0].get(), TokenType::Star,
                    [](auto left) {
                    ExpectGrouping(left, [](auto inner) {
                        ExpectBinary(inner, TokenType::Plus,
                            [](auto l) { ExpectNumber(l, "1"); },
                            [](auto r) { ExpectNumber(r, "2"); });
                        });
                    },
                    [](auto right) { ExpectNumber(right, "3"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_binary_rejected_after_grouping_closes",
            "let a = (1\n"
            "+ 2)\n"
            "+ 3\n",
            { {Err::InvalidStandaloneStatement, 3, 3} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, [](auto inner) {
                    ExpectBinary(inner, TokenType::Plus,
                        [](auto l) { ExpectNumber(l, "1"); },
                        [](auto r) { ExpectNumber(r, "2"); });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "unary_operator_spanning_newline_is_allowed",
            "let a = -\n"
            "2\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                auto unary = dynamic_cast<const UnaryExpression*>(expr);
                ASSERT_NE(unary, nullptr);
                EXPECT_EQ(unary->op, TokenType::Minus);
                ExpectNumber(unary->right.get(), "2");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "multiline_chained_comparisons_inside_grouping",
            "let a = (\n"
            "1 <\n"
            "2 <\n"
            "3\n"
            ")\n",
            { {Err::ChainingNotAllowedForComparisonOperations, 3, 3} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "precedence_inversion_sanity_check_inside_multiline",
            "let a = (1\n+ 2 * 3)\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, [](auto inner) {
                    ExpectBinary(inner, TokenType::Plus,
                        [](auto l) { ExpectNumber(l, "1"); },
                        [](auto r) {
                        ExpectBinary(r, TokenType::Star,
                            [](auto rl) { ExpectNumber(rl, "2"); },
                            [](auto rr) { ExpectNumber(rr, "3"); });
                        });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "binary_dangling_at_eof_after_newline",
            "let a = 1\n"
            "*",
            { {Err::InvalidExpression, 2, 1} },
            VerifyAssignmentValue([](auto expr) {
                ExpectNumber(expr, "1");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "return_statement_ignores_multiline_binary",
            "func test() -> int {\n"
            "  return 1\n"
            "  * 2\n"
            "}\n",
            { {Err::InvalidExpression, 3, 3} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->body.size(), 1);

            auto ret = dynamic_cast<ReturnStatement*>(ast.function_definitions[0]->body[0].get());
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(ret->values.size(), 1);
            ExpectNumber(ret->values[0].get(), "1");
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
