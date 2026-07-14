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
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}