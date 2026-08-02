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
                .test_name = "reassignment_using_declaration_keyword",
                .source_code = "func f() -> void {\n  let = 5\n}\n",
                .expected_errors = {
                    {.code = Err::ExpectedIdentifier, .line = 2, .column = 7}
                },
                .verify_ast = [](const Program& ast) {
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
                .test_name = "top_level_reassignment_using_declaration_keyword",
                .source_code = "let = 5",
                .expected_errors = {
                    {.code = Err::ExpectedIdentifier, .line = 1, .column = 5}
                },
                .verify_ast = [](const Program& ast) {
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
                .test_name = "reassignment_self_missing_property_name",
                .source_code = "self. = 42\nlet c = 2\n",
                .expected_errors = {
                    {.code = Err::ExpectedPropertyName, .line = 1, .column = 6}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
                    ASSERT_NE(assign_c, nullptr);
                    EXPECT_EQ(assign_c->targets[0].name, "c");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "reassignment_self_bracket_unexpected_comma",
                .source_code = "self[0, ] = 42\nlet c = 2\n",
                .expected_errors = {
                    {.code = Err::UnexpectedCommaInBracketAccess, .line = 1, .column = 7}
                },
                .verify_ast = [](const Program& ast) {
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