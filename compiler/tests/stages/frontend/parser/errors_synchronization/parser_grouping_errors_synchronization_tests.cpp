#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

using Err = valuascript::compiler::ValuascriptErrorCode;

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
        "grouping_unclosed",
        "let a = ( 1 + 2 \n"
        "let recovery = 1\n",
        { {Err::ExpectedRightParenAfterExpression, 1, 16} },
        [](const Program& ast) {
        ExpectGroupingLiteral(ast);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_empty_is_actually_a_tuple",
        "let a = ()\n"
        "let recovery = 1\n",
        {},
        [](const Program& ast) {
        auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
        ASSERT_NE(dynamic_cast<TupleLiteral*>(assign->value.get()), nullptr);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_with_garbage_expression",
        "let a = ( * )\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 11} },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_missing_operator_inside",
        "let a = ( 1 2 )\n"
        "let recovery = 1\n",
        { {Err::MissingOperatorInsideGrouping, 1, 13} },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_with_trailing_binary_op",
        "let a = ( 1 + )\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 15} },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_recovery_with_trailing_math",
        "let a = ( 1 + * ) + 5\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 15} },
        [](const Program& ast) {
        auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
        auto *binary = dynamic_cast<BinaryExpression*>(assign->value.get());
        ASSERT_NE(binary, nullptr) << "Should have parsed the outer binary expression (+ 5)";
        EXPECT_EQ(binary->op, TokenType::Plus);
        ASSERT_NE(dynamic_cast<GroupingExpression*>(binary->left.get()), nullptr);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_nested_recovery",
        "let a = ( ( * ) + 1 )\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 13} },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_closed_with_wrong_bracket",
        "let a = ( 1 + 2 ]\n"
        "let recovery = 1\n",
        { {Err::ExpectedRightParenAfterExpression, 1, 17} },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_mismatched_bracket_in_nested_list",
        "let a = [ ( 1 + 2 ], 3 ]\n"
        "let recovery = 1\n",
        { {Err::ExpectedRightParenAfterExpression, 1, 19} },
        [](const Program& ast) {
        auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
        auto *tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
        ASSERT_NE(tensor, nullptr);
        EXPECT_EQ(tensor->elements.size(), 2);
        EXPECT_NE(dynamic_cast<GroupingExpression*>(tensor->elements[0].get()), nullptr);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_with_top_level_declaration",
        "let a = ( let x = 1 )\n"
        "let recovery = 1\n",
        {
        {Err::TopLevelDeclarationNotAllowedHere, 1, 11},
        },
        [](const Program& ast) {
        EXPECT_EQ(ast.execution_steps.size(), 2);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_eof_mid_expression",
        "let a = ( 1 + ",
        { {Err::InvalidExpression, 1, 14}, {Err::ExpectedRightParenAfterExpression, 1, 14} },
        [](const Program& ast) {
        EXPECT_EQ(ast.execution_steps.size(), 1);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "ambiguity_starts_as_grouping_fails_to_tuple",
        "let a = ( 1 + * , 2 )\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 15} },
        [](const Program& ast) {
        auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
        auto *tuple = dynamic_cast<TupleLiteral*>(assign->value.get());
        ASSERT_NE(tuple, nullptr);
        EXPECT_EQ(tuple->elements.size(), 1);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_leading_comma_error",
        "let a = ( , 1 )\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 11} },
        [](const Program& ast) {
        auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
        ASSERT_NE(dynamic_cast<TupleLiteral*>(assign->value.get()), nullptr);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_recursive_unclosed",
        "let a = (((1 + 2\n"
        "let recovery = 1\n",
        {
        {Err::ExpectedRightParenAfterExpression, 1, 17},
        {Err::ExpectedRightParenAfterExpression, 1, 17},
        {Err::ExpectedRightParenAfterExpression, 1, 17}
        },
        [](const Program& ast) {
        auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
        auto* g1 = dynamic_cast<GroupingExpression*>(assign->value.get());
        auto* g2 = dynamic_cast<GroupingExpression*>(g1->expression.get());
        auto* g3 = dynamic_cast<GroupingExpression*>(g2->expression.get());
        ASSERT_NE(g3, nullptr);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_multiple_inner_errors",
        "let a = ( * + * )\n"
        "let recovery = 1\n",
        {
        {Err::InvalidExpression, 1, 11},
        },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_broken_with_call",
        "let a = ( * )()\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 11} },
        [](const Program& ast) {
        auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
        auto* call = dynamic_cast<FunctionCall*>(assign->value.get());
        ASSERT_NE(call, nullptr);
        ASSERT_NE(dynamic_cast<GroupingExpression*>(call->target.get()), nullptr);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_broken_with_dot_access",
        "let a = ( * ).property\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 11} },
        [](const Program& ast) {
        auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
        auto* dot = dynamic_cast<DotAccess*>(assign->value.get());
        ASSERT_NE(dot, nullptr);
        EXPECT_EQ(dot->property_name, "property");
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_interrupted_by_directive",
        "let a = ( 1 + #hidden )\n"
        "let recovery = 1\n",
        {
        {Err::TopLevelDeclarationNotAllowedHere, 1, 15},
        {Err::InvalidExpression, 1, 23}
        },
        [](const Program& ast) {
        EXPECT_EQ(ast.execution_steps.size(), 2);
        EXPECT_EQ(ast.directives.size(), 0);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_mismatched_closer_no_comma_heuristic",
        "let a = ( 1 + 2 }\n"
        "let recovery = 1\n",
        { {Err::ExpectedRightParenAfterExpression, 1, 17} },
        ExpectGrouping()
        },
        ParserErrorsSynchronizationTestCase{
        "grouping_with_reserved_keyword_as_identifier",
        "let a = ( let )\n"
        "let recovery = 1\n",
        { {Err::ReservedKeywordAsIdentifier, 1, 11} },
        [](const Program& ast) {
        auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
        auto* group = dynamic_cast<GroupingExpression*>(assign->value.get());
        ASSERT_NE(dynamic_cast<IdentifierAccess*>(group->expression.get()), nullptr);
        }
        }
    ),
    [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
    return info.param.test_name;
    }
);
