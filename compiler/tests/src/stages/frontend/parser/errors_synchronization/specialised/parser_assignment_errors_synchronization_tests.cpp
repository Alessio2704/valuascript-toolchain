#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    class AssignmentParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(AssignmentParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        AssignmentErrors,
        AssignmentParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "missing_value_after_equals_eof",
            "let a =",
            {
            {Err::MissingValueAfterEquals, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets.size(), 1);
            EXPECT_EQ(assign->targets[0].first, "a");
            EXPECT_EQ(assign->value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_value_after_equals_next_stmt",
            "let a =\n"
            "let b = 2\n",
            {
            {Err::MissingValueAfterEquals, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            EXPECT_EQ(assign_a->targets.size(), 1);
            EXPECT_EQ(assign_a->targets[0].first, "a");
            EXPECT_EQ(assign_a->value, nullptr);

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets.size(), 1);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            EXPECT_NE(assign_b->value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "incomplete_assignment_eof",
            "let a",
            {
            {Err::IncompleteAssignment, 1, 6},
            {Err::MissingValueAfterEquals, 1, 6}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets.size(), 1);
            EXPECT_EQ(assign->targets[0].first, "a");
            EXPECT_EQ(assign->value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "incomplete_assignment_next_stmt",
            "let a\n"
            "let b = 2\n",
            {
            {Err::IncompleteAssignment, 1, 6},
            {Err::MissingValueAfterEquals, 1, 6}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            EXPECT_EQ(assign_a->targets.size(), 1);
            EXPECT_EQ(assign_a->targets[0].first, "a");
            EXPECT_EQ(assign_a->value, nullptr);

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "incomplete_assignment_with_expr",
            "let a 1\n"
            "let b = 2\n",
            {
            {Err::IncompleteAssignment, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_comma_in_multi_assignment",
            "let a b = 1\n"
            "let c = 2\n",
            {
            {Err::ExpectedCommaInMultiAssignment, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets.size(), 2);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifiers_on_multi_assignment_single_elements",
            "let a, @modifier b = 1\n",
            {
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 2);
            EXPECT_EQ(assign->targets[0].first, "a");
            EXPECT_EQ(assign->targets[1].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multi_reassignment_not_supported",
            "a, b = 1\n"
            "let c = 2\n",
            {
            {Err::MultiReassignmentNotSupported, 1, 2}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_left_side_expression_in_reassignment",
            "a + b = 3\n"
            "let c = 2\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_standalone_statement_valid_expression",
            "a + b\n"
            "let c = 2\n",
            {
            {Err::InvalidStandaloneStatement, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_operator_in_reassignment_value",
            "a = 1 2\n"
            "let c = 2\n",
            {
            {Err::MissingOperator, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_identifier_in_assignment",
            "let 1 = 2\n"
            "let c = 2\n",
            {
            {Err::InvalidIdentifier, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets[0].first, "<error>");
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_value_after_equals_reassignment_eof",
            "a = ",
            {
            {Err::MissingValueAfterEquals, 1, 4}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);
            ASSERT_NE(reassign->target, nullptr);
            auto target = dynamic_cast<IdentifierAccess*>(reassign->target.get());
            ASSERT_EQ(target->name, "a");
            ASSERT_EQ(reassign->value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_value_after_equals_reassignment_next_stmt",
            "a =\n"
            "let c = 2\n",
            {
            {Err::MissingValueAfterEquals, 1, 4}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);

            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);
            ASSERT_NE(reassign->target, nullptr);
            auto target = dynamic_cast<IdentifierAccess*>(reassign->target.get());
            ASSERT_EQ(target->name, "a");
            ASSERT_EQ(reassign->value, nullptr);

            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_chained_comparison_error",
            "a = 1 < 2 < 3\n"
            "let c = 2\n",
            {
            {Err::ChainingNotAllowedForComparisonOperations, 1, 11}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "let_missing_value",
            "let a : int =\n"
            "let b = 2\n",
            {
            {Err::MissingValueAfterEquals, 1, 14}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            EXPECT_NE(assign_a->targets[0].second, nullptr);
            EXPECT_EQ(assign_a->targets[0].second->name, "int");
            EXPECT_EQ(assign_a->value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_value_in_bracket_reassignment",
            "a[0] =\n"
            "let c = 2\n",
            {
            {Err::MissingValueAfterEquals, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);

            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);
            ASSERT_NE(reassign->target, nullptr);
            auto target_1 = dynamic_cast<BracketAccess*>(reassign->target.get());
            auto target_2 = dynamic_cast<IdentifierAccess*>(target_1->target.get());
            ASSERT_EQ(target_2->name, "a");
            ASSERT_EQ(reassign->value, nullptr);


            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_left_side_expression_function_call",
            "a() = 1\n"
            "let c = 2\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifiers_on_reassignment",
            "@modifier a = 1\n",
            {
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 1}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifiers_on_invalid_standalone_statement",
            "@modifier 1 + 2\n"
            "let c = 2\n",
            {
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 1},
            {Err::InvalidStandaloneStatement, 1, 15},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_top_level_binary_expression",
            "1 + 2\n"
            "let c = 2\n",
            {
            {Err::InvalidStandaloneStatement, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_multiple_garbage_modifiers",
            "@mod1 @123 let a = 1\n"
            "let c = 2\n",
            {
            {Err::ExpectedModifierName, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "a");
            EXPECT_EQ(assign->modifiers.size(), 2);
            EXPECT_EQ(assign->modifiers[0].name, "mod1");
            EXPECT_EQ(assign->modifiers[1].name, "<error>");
            auto assign_2 = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_2, nullptr);
            EXPECT_EQ(assign_2->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "block_invalid_left_side_expression_in_reassignment",
            "func test() -> void {\n"
            "  1 + 2 = 3\n"
            "}\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 9}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "block_invalid_standalone_statement",
            "func test() -> void {\n"
            "  1 + 2\n"
            "}\n",
            {
            {Err::InvalidStandaloneStatement, 2, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "block_multi_reassignment_not_supported",
            "func test() -> void {\n"
            "  1, 2 = 3\n"
            "}\n",
            {
            {Err::MultiReassignmentNotSupported, 2, 4}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_missing_type_annotation_after_colon",
            "let a: = 1\n"
            "let b: int = 2\n",
            {
            {Err::MissingTypeAnnotation, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets[0].first, "a");
            EXPECT_EQ(assign_1->targets[0].second.get(), nullptr);
            auto assign_2 = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_2, nullptr);
            EXPECT_EQ(assign_2->targets[0].first, "b");
            EXPECT_NE(assign_2->targets[0].second.get(), nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_multiple_commas_in_targets",
            "let a,, b = 1\n"
            "let c = 2\n",
            {
            {Err::InvalidIdentifier, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets.size(), 3);
            EXPECT_EQ(assign_1->targets[0].first, "a");
            EXPECT_EQ(assign_1->targets[1].first, "<error>");
            EXPECT_EQ(assign_1->targets[2].first, "b");
            auto assign_2 = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_2, nullptr);
            EXPECT_EQ(assign_2->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_tuple_1",
            "(a, b) = 1\n"
            "let c = 2\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_tuple_2",
            "func f() -> void {\n"
            "  (a, b) = 1\n"
            "}\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 10}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_tensor_1",
            "[1, 2] = 3\n"
            "let c = 2\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 8}
            },
            [](const Program& ast) {
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_tensor_2",
            "func f() ->  void {\n"
            "  [1, 2] = 3\n"
            "}\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 10}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifiers_on_reassignment_with_property_access",
            "@modifier a.b = 1\n"
            "let c = 2\n",
            {
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 1}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_block_level_incomplete_value",
            "func f() -> void {\n"
            "  let a =\n"
            "  let b = 2\n"
            "}\n",
            {
            {Err::MissingValueAfterEquals, 2, 10}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 2);

            auto assign_a = dynamic_cast<Assignment*>(func->body[0].get());
            ASSERT_NE(assign_a, nullptr);
            EXPECT_EQ(assign_a->targets[0].first, "a");
            EXPECT_EQ(assign_a->value, nullptr);

            auto assign_b = dynamic_cast<Assignment*>(func->body[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_missing_value_after_equals_block",
            "func f() -> void {\n"
            "  a =\n"
            "  b = 2\n"
            "}\n",
            {
            {Err::MissingValueAfterEquals, 2, 6}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 2);

            auto reassign_a = dynamic_cast<Reassignment*>(func->body[0].get());
            ASSERT_NE(reassign_a, nullptr);
            auto reassign_a_id = dynamic_cast<IdentifierAccess*>(reassign_a->target.get());
            ASSERT_NE(reassign_a_id, nullptr);
            ASSERT_EQ(reassign_a_id->name, "a");
            ASSERT_EQ(reassign_a->value, nullptr);

            auto reassign_b = dynamic_cast<Reassignment*>(func->body[1].get());
            ASSERT_NE(reassign_b, nullptr);
            auto reassign_b_id = dynamic_cast<IdentifierAccess*>(reassign_b->target.get());
            ASSERT_NE(reassign_b_id, nullptr);
            ASSERT_EQ(reassign_b_id->name, "b");
            ASSERT_NE(reassign_b->value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_reserved_keyword_target",
            "let true = 1\n"
            "let b = 2\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_true = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_true, nullptr);
            EXPECT_EQ(assign_true->targets[0].first, "true");

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multi_assignment_with_broken_type_recovery",
            "let a: map<*, int>, b = 1\n"
            "let c = 2\n",
            {
            {Err::MissingTypeAnnotation, 1, 12}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_ab = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_ab, nullptr);
            ASSERT_EQ(assign_ab->targets.size(), 2);
            EXPECT_EQ(assign_ab->targets[0].first, "a");
            EXPECT_EQ(assign_ab->targets[1].first, "b");

            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_tuple",
            "func f() -> void {\n"
            "  (a, b) = (1, 2)\n"
            "}\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 10}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_tensor",
            "func f() -> void {\n"
            "  [x, y] = [3, 4]\n"
            "}\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 10}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "chained_reassignment_not_supported",
            "func f() -> void {\n"
            "  a = b = c = 0\n"
            "}\n",
            {
            {Err::InvalidExpression, 2, 9}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 1);

            auto reassign = dynamic_cast<Reassignment*>(func->body[0].get());
            ASSERT_NE(reassign, nullptr);

            auto target = dynamic_cast<IdentifierAccess*>(reassign->target.get());
            ASSERT_NE(target, nullptr);
            ASSERT_EQ(target->name, "a");

            auto value = dynamic_cast<IdentifierAccess*>(reassign->value.get());
            ASSERT_NE(value, nullptr);
            ASSERT_EQ(value->name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_boolean",
            "func f() -> void {\n"
            "  true = false\n"
            "}\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_using_declaration_keyword",
            "func f() -> void {\n"
            "  let = 5\n"
            "}\n",
            {
            {Err::InvalidIdentifier, 2, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 1);
            auto assign_1 = dynamic_cast<Assignment*>(func->body[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets[0].first, "<error>");
            EXPECT_EQ(assign_1->targets[0].second.get(), nullptr);
            auto assign_1_value = dynamic_cast<NumberLiteral*>(assign_1->value.get());
            EXPECT_EQ(assign_1_value->value, "5");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "top_level_reassignment_invalid_lvalue_tuple",
            "(a, b) = (1, 2)",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "top_level_reassignment_invalid_lvalue_tensor",
            "[x, y] = [3, 4]",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 8}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "top_level_chained_reassignment_not_supported",
            "a = b = c = 0",
            {
            {Err::InvalidExpression, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);

            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);

            auto target = dynamic_cast<IdentifierAccess*>(reassign->target.get());
            ASSERT_NE(target, nullptr);
            ASSERT_EQ(target->name, "a");

            auto value = dynamic_cast<IdentifierAccess*>(reassign->value.get());
            ASSERT_NE(value, nullptr);
            ASSERT_EQ(value->name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "top_level_reassignment_invalid_lvalue_boolean",
            "true = false",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 6}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "top_level_reassignment_using_declaration_keyword",
            "let = 5",
            {
            {Err::InvalidIdentifier, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets[0].first, "<error>");
            EXPECT_EQ(assign_1->targets[0].second.get(), nullptr);
            auto assign_1_value = dynamic_cast<NumberLiteral*>(assign_1->value.get());
            EXPECT_EQ(assign_1_value->value, "5");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_invalid_lvalue_self_standalone",
            "self = 42\n"
            "let c = 2\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 6}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_self_missing_property_name",
            "self. = 42\n"
            "let c = 2\n",
            {
            {Err::ExpectedPropertyName, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_self_bracket_unexpected_comma",
            "self[0, ] = 42\n"
            "let c = 2\n",
            {
            {Err::UnexpectedCommaInBracketAccess, 1, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);

            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);

            auto bracket = dynamic_cast<BracketAccess*>(reassign->target.get());
            ASSERT_NE(bracket, nullptr);
            ASSERT_NE(dynamic_cast<SelfExpression*>(bracket->target.get()), nullptr);
            EXPECT_EQ(bracket->index, nullptr) << "Index should be null due to local comma error recovery";

            auto val = dynamic_cast<NumberLiteral*>(reassign->value.get());
            ASSERT_NE(val, nullptr);
            EXPECT_EQ(val->value, "42");

            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_self_method_call_invalid_lvalue",
            "self.calc() = 42\n"
            "let c = 2\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 1, 13}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_self_target_missing_property",
            "let a = self.\n"
            "let c = 2\n",
            {
            {Err::ExpectedPropertyName, 1, 14}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_self_valid_target_missing_value",
            "self.counter =\n"
            "let c = 2\n",
            {
            {Err::MissingValueAfterEquals, 1, 15}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);

            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);

            auto dot = dynamic_cast<DotAccess*>(reassign->target.get());
            ASSERT_NE(dot, nullptr);
            ASSERT_NE(dynamic_cast<SelfExpression*>(dot->target.get()), nullptr);
            EXPECT_EQ(dot->property_name, "counter");

            EXPECT_EQ(reassign->value, nullptr);

            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_self_chained_invalid",
            "self.a = self.b = 10\n"
            "let c = 2\n",
            {
            {Err::InvalidExpression, 1, 17}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);

            auto reassign = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassign, nullptr);

            auto target_dot = dynamic_cast<DotAccess*>(reassign->target.get());
            ASSERT_NE(target_dot, nullptr);
            EXPECT_EQ(target_dot->property_name, "a");
            ASSERT_NE(dynamic_cast<SelfExpression*>(target_dot->target.get()), nullptr);

            auto val_dot = dynamic_cast<DotAccess*>(reassign->value.get());
            ASSERT_NE(val_dot, nullptr);
            EXPECT_EQ(val_dot->property_name, "b");
            ASSERT_NE(dynamic_cast<SelfExpression*>(val_dot->target.get()), nullptr);

            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "block_reassignment_self_missing_value",
            "func f() -> void {\n"
            "  self.state =\n"
            "  let b = 2\n"
            "}\n",
            {
            {Err::MissingValueAfterEquals, 2, 15}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 2);

            auto reassign = dynamic_cast<Reassignment*>(func->body[0].get());
            ASSERT_NE(reassign, nullptr);

            auto dot = dynamic_cast<DotAccess*>(reassign->target.get());
            ASSERT_NE(dot, nullptr);
            ASSERT_NE(dynamic_cast<SelfExpression*>(dot->target.get()), nullptr);
            EXPECT_EQ(dot->property_name, "state");
            EXPECT_EQ(reassign->value, nullptr);

            auto assign_b = dynamic_cast<Assignment*>(func->body[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "assignment_self_as_target_error",
            "let self = 10\n"
            "let recovery = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 5} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            EXPECT_EQ(assign->targets[0].first, "self");
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
