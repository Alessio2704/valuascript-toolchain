#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const SwitchExpression* ExpectRecoveredSwitch(const Program& ast)
        {
            EXPECT_GE(ast.execution_steps.size(), 1) << "Expected an assignment at the top level.";
            if (ast.execution_steps.empty()) return nullptr;

            auto assign = dynamic_cast<const Assignment*>(ast.execution_steps[0].get());
            if (!assign) return nullptr;

            auto switch_expr = dynamic_cast<const SwitchExpression*>(assign->value.get());
            return switch_expr;
        }
    }

    class SwitchParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(SwitchParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        SwitchStressTest,
        SwitchParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "nested_switch_steals_outer_cases_on_missing_brace",
                .source_code = "let x = switch(v) {\n    case A -> switch(y) {\n        case B -> 1\n    case C -> 2\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 5, .column = 1}
                },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1) << "Outer switch should only have 1 case.";

                    auto inner_sw = dynamic_cast<const SwitchExpression*>(sw->cases[0].result.get());
                    ASSERT_NE(inner_sw, nullptr);
                    EXPECT_EQ(inner_sw->cases.size(), 2) << "Inner switch should have stolen the second case.";
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "unclosed_switch_inside_grouping_escapes_properly",
                .source_code = "let x = (switch(v) { case A -> 1) + 2\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 1, .column = 32}
                },
                .verify_ast = [](const Program &ast) {
                    EXPECT_GE(ast.execution_steps.size(), 1);
                    auto last_stmt = dynamic_cast<const Assignment*>(ast.execution_steps.back().get());
                    ASSERT_NE(last_stmt, nullptr);
                    EXPECT_EQ(last_stmt->targets[0].name, "a");
                }
            }
        ),
        TestNameGenerator{}
    );
}
