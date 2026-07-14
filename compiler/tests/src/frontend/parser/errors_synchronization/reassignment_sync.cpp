#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    class AssignmentParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(AssignmentParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        AssignmentErrors,
        AssignmentParserSynchronizationTest,
        ::testing::Values(
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
            EXPECT_EQ(assign->targets[0].name, "c");
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
            EXPECT_EQ(assign->targets[0].name, "c");
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
            {Err::InvalidStandaloneStatement, 1, 11, 1, 16},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
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
            EXPECT_EQ(assign_a->targets[0].name, "a");
            EXPECT_EQ(assign_a->value, nullptr);

            auto assign_b = dynamic_cast<Assignment*>(func->body[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].name, "b");
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
            EXPECT_EQ(assign_ab->targets[0].name, "a");
            EXPECT_EQ(assign_ab->targets[1].name, "b");

            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].name, "c");
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
            "reassignment_using_declaration_keyword",
            "func f() -> void {\n"
            "  let = 5\n"
            "}\n",
            {
            {Err::ExpectedIdentifier, 2, 7}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->body.size(), 1);
            auto assign_1 = dynamic_cast<Assignment*>(func->body[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets[0].name, "<error>");
            EXPECT_EQ(assign_1->targets[0].type.get(), nullptr);
            auto assign_1_value = dynamic_cast<NumberLiteral*>(assign_1->value.get());
            EXPECT_EQ(assign_1_value->value, "5");
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
            "top_level_reassignment_using_declaration_keyword",
            "let = 5",
            {
            {Err::ExpectedIdentifier, 1, 5}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_1, nullptr);
            EXPECT_EQ(assign_1->targets[0].name, "<error>");
            EXPECT_EQ(assign_1->targets[0].type.get(), nullptr);
            auto assign_1_value = dynamic_cast<NumberLiteral*>(assign_1->value.get());
            EXPECT_EQ(assign_1_value->value, "5");
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
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].name, "c");
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
            EXPECT_NE(bracket->index, nullptr) << "Index should be preserved despite local comma error recovery";

            auto val = dynamic_cast<NumberLiteral*>(reassign->value.get());
            ASSERT_NE(val, nullptr);
            EXPECT_EQ(val->value, "42");

            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].name, "c");
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
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_c, nullptr);
            EXPECT_EQ(assign_c->targets[0].name, "c");
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
            EXPECT_EQ(assign_c->targets[0].name, "c");
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
            EXPECT_EQ(assign_c->targets[0].name, "c");
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
            EXPECT_EQ(assign_b->targets[0].name, "b");
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}