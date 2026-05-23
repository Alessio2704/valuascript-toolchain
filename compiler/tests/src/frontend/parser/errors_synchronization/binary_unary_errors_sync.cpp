#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "frontend/parser/ast.h"

namespace valuascript::compiler::test
{
    namespace
    {
        void ExpectNumber(const Expression* expr, const std::string& value)
        {
            auto num = dynamic_cast<const NumberLiteral*>(expr);
            ASSERT_NE(num, nullptr) << "Expected NumberLiteral, but got " << (expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(num->value, value);
        }

        void ExpectBinary(const Expression* expr, TokenType expected_op,
                          const std::function<void(const Expression*)>& left_verifier,
                          const std::function<void(const Expression*)>& right_verifier)
        {
            auto bin = dynamic_cast<const BinaryExpression*>(expr);
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

        void ExpectGrouping(const Expression* expr, const std::function<void(const Expression*)>& inner_verifier)
        {
            auto grp = dynamic_cast<const GroupingExpression*>(expr);
            ASSERT_NE(grp, nullptr) << "Expected GroupingExpression, but got " << (expr
                    ? typeid(*expr).name()
                    : "nullptr");

            if (inner_verifier) inner_verifier(grp->expression.get());
            else
                EXPECT_EQ(grp->expression, nullptr);
        }

        auto VerifyAssignmentValue(const std::function<void(const Expression*)>& value_verifier,
                                   size_t expected_steps = 1,
                                   size_t step_index = 0)
        {
            return [=](const Program& ast)
            {
                ASSERT_EQ(ast.execution_steps.size(), expected_steps) << "Execution step count mismatch";
                auto assign = dynamic_cast<Assignment*>(ast.execution_steps[step_index].get());
                ASSERT_NE(assign, nullptr) << "Expected step " << step_index << " to be an Assignment";
                value_verifier(assign->value.get());
            };
        }
    }

    class BinaryAndUnaryErrorsParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(BinaryAndUnaryErrorsParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        BinaryAndUnaryErrorsStressTests,
        BinaryAndUnaryErrorsParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "binary_missing_left",
            "let a = * 2\n"
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
            "grouping_missing_operator",
            "let a = (1 2)\n",
            { {Err::MissingOperatorInsideGrouping, 1, 12} },
            VerifyAssignmentValue([](auto expr) {
                ExpectGrouping(expr, [](auto inner) { ExpectNumber(inner, "1"); });
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
                    ASSERT_EQ(tensor_left->elements.size(), 2);
                    ExpectNumber(tensor_left->elements[1].get(), "1");
                    },
                    [](auto right) {
                    auto tensor_right = dynamic_cast<const TensorLiteral*>(right);
                    ASSERT_NE(tensor_right, nullptr);
                    ASSERT_EQ(tensor_right->elements.size(), 2);
                    ExpectNumber(tensor_right->elements[0].get(), "2");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_standalone_statement",
            "1 + 1\n"
            "let b = 2\n",
            { {Err::InvalidStandaloneStatement, 1, 5} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
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
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}
