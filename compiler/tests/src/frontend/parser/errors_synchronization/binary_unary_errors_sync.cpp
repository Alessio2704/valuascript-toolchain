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
                .test_name = "binary_operator_dangling_on_next_line",
                .source_code = "let a = 1\n* 2\nlet b = 3\n",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 2, .column = 1} },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
                    ASSERT_NE(assign_b, nullptr);
                    EXPECT_EQ(assign_b->targets[0].name, "b");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_binary_rejected_at_top_level",
                .source_code = "let a = 1\n* 2\n",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 2, .column = 1} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectNumber(expr, "1");
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_unary_becomes_standalone_and_is_rejected",
                .source_code = "let a = 1\n+ 2\n",
                .expected_errors = { {.code = Err::InvalidStandaloneStatement, .line = 2, .column = 1, .line_end = 2, .column_end = 4} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectNumber(expr, "1");
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_operator_at_end_of_line_is_allowed",
                .source_code = "let a = 1 +\n2\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectBinary(expr, TokenType::Plus,
                        [](auto left) { ExpectNumber(left, "1"); },
                        [](auto right) { ExpectNumber(right, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_binary_allowed_inside_parentheses",
                .source_code = "let a = (1\n+ 2)\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectGrouping(expr, [](auto inner) {
                        ExpectBinary(inner, TokenType::Plus,
                            [](auto left) { ExpectNumber(left, "1"); },
                            [](auto right) { ExpectNumber(right, "2"); });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_binary_allowed_inside_tensor_literal",
                .source_code = "let a = [1\n* 2, 3]\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
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
                .test_name = "multiline_binary_with_trailing_operator_inside_dictionary",
                .source_code = "let a = {key: 1 -\n2}\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
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
                .test_name = "multiline_binary_wrapped_in_parens_inside_dictionary",
                .source_code = "let a = {key: (1\n- 2)}\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
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
                .test_name = "multiline_binary_rejected_inside_dictionary_literal",
                .source_code = "let a = {key: 1\n-2}\n",
                .expected_errors = { {.code = Err::MissingOperator, .line = 1, .column = 16} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    auto dict = dynamic_cast<const DictLiteral*>(expr);
                    ASSERT_NE(dict, nullptr);
                    ASSERT_EQ(dict->elements.size(), 1);
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_binary_not_unary_rejected_inside_dictionary_literal",
                .source_code = "let a = {key: 1\n*2}\n",
                .expected_errors = { {.code = Err::MissingOperator, .line = 1, .column = 16} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    auto dict = dynamic_cast<const DictLiteral*>(expr);
                    ASSERT_NE(dict, nullptr);
                    ASSERT_EQ(dict->elements.size(), 1);
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiline_binary_allowed_inside_function_call_arguments",
                .source_code = "let a = test(arg: 1\n/ 2)\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    auto call = dynamic_cast<const FunctionCall*>(expr);
                    ASSERT_NE(call, nullptr);
                    ASSERT_EQ(call->arguments.size(), 1);
                    ExpectBinary(call->arguments[0].second.get(), TokenType::Slash,
                        [](auto left) { ExpectNumber(left, "1"); },
                        [](auto right) { ExpectNumber(right, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "complex_multiline_mixed_groupings",
                .source_code = "let a = [\n  (1\n  + 2)\n  * 3\n]\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
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
                .test_name = "multiline_binary_rejected_after_grouping_closes",
                .source_code = "let a = (1\n+ 2)\n+ 3\n",
                .expected_errors = { {.code = Err::InvalidStandaloneStatement, .line = 3, .column = 1, .line_end = 3, .column_end = 4} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectGrouping(expr, [](auto inner) {
                        ExpectBinary(inner, TokenType::Plus,
                            [](auto l) { ExpectNumber(l, "1"); },
                            [](auto r) { ExpectNumber(r, "2"); });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "precedence_inversion_sanity_check_inside_multiline",
                .source_code = "let a = (1\n+ 2 * 3)\n",
                .expected_errors = {},
                .verify_ast = VerifyAssignmentValue([](auto expr) {
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
                .test_name = "binary_dangling_at_eof_after_newline",
                .source_code = "let a = 1\n*",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 2, .column = 1} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectNumber(expr, "1");
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "return_statement_ignores_multiline_binary",
                .source_code = "func test() -> int {\n  return 1\n  * 2\n}\n",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 3, .column = 3} },
                .verify_ast = [](const Program& ast) {
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
