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
            "tuple_missing_closing_paren",
            "let a = (1, 2 \n"
            "let recovery = 1\n",
            { {Err::ExpectedRightParenAfterTupleElements, 1, 13} },
            ExpectTuple(2)
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_missing_multiple_closing_paren",
            "let a = (1, 2 \n"
            "let b = (3, 4 \n"
            "let c = (5, 6 \n"
            "let recovery = 1\n",
            {
            {Err::ExpectedRightParenAfterTupleElements, 1, 13},
            {Err::ExpectedRightParenAfterTupleElements, 2, 13},
            {Err::ExpectedRightParenAfterTupleElements, 3, 13}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 4);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "array_inside_tuple_first_element_error",
            "let a = ([1, 2)\n"
            "let recovery = 1\n",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 14} },
            [](const Program& ast) {
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
            "array_inside_tuple_second_element_error",
            "let a = (1, [1, 2)\n"
            "let recovery = 1\n",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 17} },
            ExpectTuple(2)
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_total_mangle",
            "let a = (1, let y = 2\n"
            "let recovery = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 13},
            {Err::ExpectedRightParenAfterTupleElements, 1, 21}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            const auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            const auto assign_1_val = dynamic_cast<TupleLiteral*>(assign_1->value.get());
            ASSERT_NE(assign_1_val, nullptr);
            ASSERT_EQ(assign_1_val->elements.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_with_top_level_declaration_inside",
            "let a = (1, let b = 2, 3)\n"
            "let recovery = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 13}
            },
            ExpectTuple(3)
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_eof_after_comma",
            "let a = (1, \n",
            { {Err::ExpectedRightParenAfterTupleElements, 1, 11} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1);
            const auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            const auto assign_1_val = dynamic_cast<TupleLiteral*>(assign_1->value.get());
            ASSERT_NE(assign_1_val, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_eof_after_first_element",
            "let a = (1 \n",
            { {Err::ExpectedRightParenAfterExpression, 1, 10} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1);
            const auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            const auto assign_1_val = dynamic_cast<GroupingExpression*>(assign_1->value.get());
            ASSERT_NE(assign_1_val, nullptr);
            ASSERT_NE(assign_1_val->expression, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_closed_with_wrong_bracket",
            "let a = (1, 2]\n"
            "let recovery = 1\n",
            { {Err::ExpectedRightParenAfterTupleElements, 1, 13} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_with_mixed_closing_brackets_inside",
            "let a = (1,[2, 3}, 4)\n"
            "let recovery = 1\n",
            {
            {Err::UnmatchedBracketAfterTensorElements, 1, 16}
            },
            ExpectTuple(3)
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_too_many_closing_parens",
            "let a = (1, 2)))\n"
            "let recovery = 1\n",
            {
            {Err::InvalidExpression, 1, 15}
            },
            ExpectTuple(2)
            },
            ParserErrorsSynchronizationTestCase{
            "tuple_deep_unclosed_cascade",
            "let a = (1, (2, (3, (4, \n",
            {
            {Err::ExpectedRightParenAfterTupleElements, 1, 23},
            {Err::ExpectedRightParenAfterTupleElements, 1, 23},
            {Err::ExpectedRightParenAfterTupleElements, 1, 23},
            {Err::ExpectedRightParenAfterTupleElements, 1, 23}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);

            auto tuple = dynamic_cast<TupleLiteral*>(assign->value.get());
            ASSERT_NE(tuple, nullptr);
            ASSERT_EQ(tuple->elements.size(), 2);
            },
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}
