#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        void ExpectTensorElements(const Program &ast, const std::vector<std::string> &expected_numbers) {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto *tensor = dynamic_cast<TensorLiteral *>(assign->value.get());
            ASSERT_NE(tensor, nullptr) << "Assignment value is not a TensorLiteral.";

            ASSERT_EQ(tensor->elements.size(), expected_numbers.size()) << "Tensor element count mismatch!";

            for (size_t i = 0; i < expected_numbers.size(); ++i) {
                if (!expected_numbers[i].empty()) {
                    auto *num = dynamic_cast<NumberLiteral *>(tensor->elements[i].get());
                    ASSERT_NE(num, nullptr) << "Expected number literal at index " << i;
                    EXPECT_EQ(num->value, expected_numbers[i]) << "Value mismatch at index " << i;
                }
            }
        }

        auto ExpectTensor(std::vector<std::string> numbers) {
            return [numbers = std::move(numbers)](const Program &ast) {
                ExpectTensorElements(ast, numbers);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

    class TensorLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TensorLiteralParserSynchronizationTest, SynchronizesTensorLiteralErrors) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TensorLiteralStressTests,
        TensorLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "tensor_missing_closing_bracket",
            "let a =[ 1, 2 \n"
            "let recovery = 1\n",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 14} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_missing_comma",
            "let a =[ 1 2 ]\n"
            "let recovery = 1\n",
            { {Err::MissingCommaOrOperatorBetweenExpressions, 1, 12} },
            ExpectTensor({"1", "2"})
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_garbage_element",
            "let a = [ 1, *, 3 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 14} },
            ExpectTensor({"1", "3"})
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_double_comma",
            "let a = [ 1,, 3 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 13} },
            ExpectTensor({"1", "3"})
            },
            ParserErrorsSynchronizationTestCase{
            "empty_tensor_with_garbage",
            "let a = [ * ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 11} },
            ExpectTensor({})
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_trailing_comma_allowed",
            "let a = [ 1, 2, ]\n"
            "let recovery = 1\n",
            {},
            ExpectTensor({"1", "2"})
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_closed_with_wrong_brace",
            "let a =[ 1, 2 }\n"
            "let recovery = 1\n",
            {
            {Err::UnmatchedBracketAfterTensorElements, 1, 15}
            },
            ExpectTensor({"1", "2"})
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_nested_recovery",
            "let a = [ 1, [ 2, * ], 3 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 19} },
            ExpectTensor({"1", "", "3"})
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_broken_inside_function_call",
            "let a = f(a: [ 1, * ], b: 2)\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 19} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto func_call = dynamic_cast<FunctionCall*>(assign->value.get());
            ASSERT_NE(func_call, nullptr);
            ASSERT_EQ(func_call->arguments.size(), 2);
            EXPECT_EQ(func_call->arguments[1].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_broken_with_postfix_access",
            "let a = [ 1, * ][0]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 14} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto bracket_access = dynamic_cast<BracketAccess*>(assign->value.get());
            ASSERT_NE(bracket_access, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_multiple_garbage_elements",
            "let a = [ *, *, 3 ]\n"
            "let recovery = 1\n",
            {
            {Err::InvalidExpression, 1, 11},
            {Err::InvalidExpression, 1, 14}
            },
            ExpectTensor({ "3" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_dict_inside_broken",
            "let a = [ { x: * }, 2 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 16} },
            ExpectTensor({ "", "2" }) // Empty string represents the failed dict node
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_switch_inside_broken",
            "let a = [ switch(x) { case a -> * }, 2 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 33} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            EXPECT_EQ(tensor->elements.size(), 2);
            auto const switch_expr = dynamic_cast<SwitchExpression*>(tensor->elements[0].get());
            EXPECT_NE(switch_expr, nullptr);
            EXPECT_EQ(switch_expr->cases.size(), 1);
            EXPECT_EQ(switch_expr->cases[0].second.get(), nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_grouping_inside_broken",
            "let a = [ (1 + *), 2 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 16} },
            [](const Program& ast) {
            const auto assignment = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto tensor = dynamic_cast<TensorLiteral*>(assignment->value.get());
            ASSERT_EQ(tensor->elements.size(), 2);
            auto first_elem = dynamic_cast<GroupingExpression*>(tensor->elements[0].get());
            ASSERT_NE(first_elem, nullptr);
            ASSERT_EQ(first_elem->expression, nullptr);
            auto second_elem = dynamic_cast<NumberLiteral*>(tensor->elements[1].get());
            ASSERT_NE(second_elem, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_missing_bracket_before_next_stmt",
            "let a = [ 1, 2\n"
            "let b = 3\n"
            "let recovery = 1\n",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 3);
            auto* b_assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(b_assign, nullptr);
            EXPECT_EQ(b_assign->targets[0].first, "a");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_missing_comma_between_complex_expr",
            "let a = [ 1 + 2 3 + 4 ]\n"
            "let recovery = 1\n",
            { {Err::MissingCommaOrOperatorBetweenExpressions, 1, 17} },
            ExpectTensor({ "", "" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_eof_immediately_after_bracket",
            "let a = [",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 10} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_with_mismatched_nested_closer",
            "let a = [ ( 1 + 2 ], 3 ]\n"
            "let recovery = 1\n",
            { {Err::ExpectedRightParenAfterExpression, 1, 19} },
            [](const Program& ast) {
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
            "tensor_multi_line_recovery",
            "let a = [\n"
            "  1,\n"
            "  *,\n"
            "  3\n"
            "]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 3, 3} },
            ExpectTensor({ "1", "3" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_in_multi_assignment_failure",
            "let a, b = [ 1, * ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 17} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            EXPECT_EQ(assign->targets.size(), 2);
            EXPECT_EQ(assign->targets[0].first, "a");
            EXPECT_EQ(assign->targets[1].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_with_comparison_no_confusion",
            "let a = [ (x > 0), * ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 20} },
            ExpectTensor({ "" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_deep_nested_barrier_failure",
            "let a = [ { key: f(a: [1, *]) }, 2 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 27} },
            ExpectTensor({ "", "2" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_consecutive_operators",
            "let a = [ 1 + * 2, 3 ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 15} },
            ExpectTensor({ "3" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_missing_comma_and_bracket",
            "let a = [ 1 2\n"
            "let recovery = 1\n",
            {
            {Err::MissingCommaOrOperatorBetweenExpressions, 1, 13},
            {Err::UnmatchedBracketAfterTensorElements, 1, 14}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            EXPECT_EQ(tensor->elements.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_trailing_comma_garbage",
            "let a = [ 1, 2, , ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 17} },
            ExpectTensor({ "1", "2" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_empty_elements_stress",
            "let a = [ , , 1, , ]\n"
            "let recovery = 1\n",
            {
            {Err::InvalidExpression, 1, 11},
            {Err::InvalidExpression, 1, 13},
            {Err::InvalidExpression, 1, 18}
            },
            ExpectTensor({ "1" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_with_unclosed_string_element",
            "let a = [ \"unclosed, 2 ]\n"
            "let recovery = 1\n",
            {
            {Err::UnclosedString, 1, 11},
            {Err::UnmatchedBracketAfterTensorElements, 1, 25}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            EXPECT_EQ(tensor->elements.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_with_illegal_statement_inside",
            "let a = [ func f() -> void {},  1]\n"
            "let recovery = 1\n",
            { {Err::TopLevelDeclarationNotAllowedHere, 1, 11} },
            ExpectTensor({ "1" })
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_with_illegal_statement_inside_new_line",
            "let a = [ \n"
            "func f() -> void {}, \n"
            "1 \n"
            "]\n"
            "let recovery = 1\n",
            {
            {Err::UnmatchedBracketAfterTensorElements, 1, 10},
            {Err::InvalidExpression, 2, 20},
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.execution_steps.size(), 2);

            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            EXPECT_EQ(tensor->elements.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_literal_with_slice_syntax",
            "let a = [1:2, 3 ]\n"
            "let recovery = 1\n",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 11} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            EXPECT_EQ(tensor->elements.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_multiple_broken_statements_sequential",
            "let a =[\n"
            "  1,\n"
            "  +,\n"
            "  2\n"
            "]\n"
            "let b =[\n"
            "  3,\n"
            "  *,\n"
            "  4,\n"
            "  /\n"
            "]\n"
            "let recovery = [ 5, 6 ]\n",
            {
            {Err::InvalidExpression, 3, 4},
            {Err::InvalidExpression, 8, 3},
            {Err::InvalidExpression, 10, 3}
            },[](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 3) << "AST must have exactly 3 statements.";

            auto* assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            EXPECT_EQ(assign_a->targets[0].first, "a");
            auto* tensor_a = dynamic_cast<TensorLiteral*>(assign_a->value.get());
            ASSERT_NE(tensor_a, nullptr);
            EXPECT_EQ(tensor_a->elements.size(), 2);

            auto* assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            auto* tensor_b = dynamic_cast<TensorLiteral*>(assign_b->value.get());
            ASSERT_NE(tensor_b, nullptr);
            EXPECT_EQ(tensor_b->elements.size(), 2);

            auto* assign_rec = dynamic_cast<Assignment*>(ast.execution_steps[2].get());
            ASSERT_NE(assign_rec, nullptr);
            EXPECT_EQ(assign_rec->targets[0].first, "recovery");
            auto* tensor_rec = dynamic_cast<TensorLiteral*>(assign_rec->value.get());
            ASSERT_NE(tensor_rec, nullptr);
            EXPECT_EQ(tensor_rec->elements.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_multiline_deeply_nested_failures",
            "let a =[\n"
            "  (1 + *),\n"
            "  { key: * }\n"
            "]\n"
            "let b = [\n"
            "[ *, 2 ],\n"
            "  f(a: *)\n"
            "]\n"
            "let recovery = 100\n",
            {
            {Err::InvalidExpression, 2, 8},
            {Err::InvalidExpression, 3, 10},
            {Err::InvalidExpression, 6, 3},
            {Err::InvalidExpression, 7, 8}
            },[](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 3) << "AST must have exactly 3 statements.";

            auto* assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto* tensor_a = dynamic_cast<TensorLiteral*>(assign_a->value.get());
            ASSERT_NE(tensor_a, nullptr);
            EXPECT_EQ(tensor_a->elements.size(), 2);
            auto tensor_a_grouping = dynamic_cast<GroupingExpression*>(tensor_a->elements[0].get());
            ASSERT_NE(tensor_a_grouping, nullptr);
            ASSERT_EQ(tensor_a_grouping->expression, nullptr);
            auto tensor_a_dict = dynamic_cast<DictLiteral*>(tensor_a->elements[1].get());
            EXPECT_EQ(tensor_a_dict->elements.size(), 1);
            EXPECT_EQ(tensor_a_dict->elements[0].value.get(), nullptr);          auto* assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            auto* tensor_b = dynamic_cast<TensorLiteral*>(assign_b->value.get());
            ASSERT_NE(tensor_b, nullptr);
            EXPECT_EQ(tensor_b->elements.size(), 2);

            auto* inner_tensor = dynamic_cast<TensorLiteral*>(tensor_b->elements[0].get());
            ASSERT_NE(inner_tensor, nullptr) << "First element of b should be a TensorLiteral";
            EXPECT_EQ(inner_tensor->elements.size(), 1) << "Inner tensor should have recovered element '2'";

            auto* assign_rec = dynamic_cast<Assignment*>(ast.execution_steps[2].get());
            ASSERT_NE(assign_rec, nullptr);
            EXPECT_EQ(assign_rec->targets[0].first, "recovery");
            ASSERT_NE(dynamic_cast<NumberLiteral*>(assign_rec->value.get()), nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tensor_multiline_dangling_operators_hitting_newline",
            "let a =[\n"
            "  10 +\n"
            "]\n"
            "let b =[\n"
            "  20,\n"
            "  30 ==\n"
            "]\n"
            "let recovery = [ 40 ]\n",
            {
            {Err::InvalidExpression, 2, 6},
            {Err::InvalidExpression, 6, 6}
            },[](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 3) << "AST must have exactly 3 statements.";

            auto* assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto* tensor_a = dynamic_cast<TensorLiteral*>(assign_a->value.get());
            EXPECT_EQ(tensor_a->elements.size(), 0);

            auto* assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            auto* tensor_b = dynamic_cast<TensorLiteral*>(assign_b->value.get());
            EXPECT_EQ(tensor_b->elements.size(), 1);

            auto* assign_rec = dynamic_cast<Assignment*>(ast.execution_steps[2].get());
            auto* tensor_rec = dynamic_cast<TensorLiteral*>(assign_rec->value.get());
            EXPECT_EQ(tensor_rec->elements.size(), 1);
            }
            }
        ), [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
