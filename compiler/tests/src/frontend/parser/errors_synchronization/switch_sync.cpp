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

        auto ExpectSwitchCases(size_t expected_cases, bool expected_has_default)
        {
            return [expected_cases, expected_has_default](const Program& ast)
            {
                auto sw = ExpectRecoveredSwitch(ast);
                ASSERT_NE(sw, nullptr) << "Switch expression not found in the first statement!";
                EXPECT_EQ(sw->cases.size(), expected_cases) << "Switch case count mismatch!";

                if (expected_has_default)
                {
                    EXPECT_NE(sw->default_case, nullptr) << "Expected default case, but got nullptr!";
                }
                else
                {
                    EXPECT_EQ(sw->default_case, nullptr) << "Expected no default case, but got one!";
                }
            };
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
                .test_name = "missing_closing_brace_escapes_to_top_level",
                .source_code = "let x = switch(v) {\n    case A -> 1\nfunc top_level() -> void {}\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 2, .column = 15} },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1);

                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->name, "top_level");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "nested_struct_without_closing_brace_escapes_to_top_level",
                .source_code = "let x = switch(v) {\n    case A -> 1\n    struct TopLevel { id: int }\nlet c = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 2, .column = 15} },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1);

                    ASSERT_EQ(ast.struct_definitions.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->name, "TopLevel");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "nested_enum_without_closing_brace_escapes_to_top_level",
                .source_code = "let x = switch(v) {\n    case A -> 1\n    enum TopLevel: int { A = 1 }\nlet c = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 2, .column = 15} },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1);

                    ASSERT_EQ(ast.enum_definitions.size(), 1);
                    EXPECT_EQ(ast.enum_definitions[0]->name, "TopLevel");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "garbage_statements_between_cases",
                .source_code = "let x = switch(v) {\n    case A -> 1\n    let y = 2\n    foo()\n    case B -> 3\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 3, .column = 5}
                },
                .verify_ast = ExpectSwitchCases(2, false)
            },
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
                .test_name = "illegal_nested_func_in_closed_switch",
                .source_code = "let x = switch(v) {\n    case A -> 1\n    func nested() -> void {}\n    case B -> 2\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 3, .column = 5}
                },
                .verify_ast = ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "illegal_nested_struct_in_closed_switch",
                .source_code = "let x = switch(v) {\n    case A -> 1\n    struct Nested { id: int }\n    case B -> 2\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 3, .column = 5}
                },
                .verify_ast = ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "illegal_directive_in_closed_switch",
                .source_code = "let x = switch(v) {\n    case A -> 1\n    #pragma = 1\n    case B -> 2\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 3, .column = 5}
                },
                .verify_ast = ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "illegal_nested_func_escapes_unclosed_switch",
                .source_code = "let x = switch(v) {\n    case A -> 1\nfunc top_level() -> void {}\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 2, .column = 15}
                },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1);

                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->name, "top_level");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "illegal_directive_escapes_unclosed_switch",
                .source_code = "let x = switch(v) {\n    case A -> 1\n#pragma = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 2, .column = 15}
                },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1);

                    ASSERT_EQ(ast.directives.size(), 1);
                    EXPECT_EQ(ast.directives[0]->name, "pragma");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "modifier_on_escaped_declaration_unclosed_switch",
                .source_code = "let x = switch(v) {\n    case A -> 1\n@export struct TopLevel { id: int }\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 2, .column = 15}
                },
                .verify_ast = [](const Program &ast) {
                    auto sw = ExpectRecoveredSwitch(ast);
                    ASSERT_NE(sw, nullptr);
                    EXPECT_EQ(sw->cases.size(), 1);

                    ASSERT_EQ(ast.struct_definitions.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->name, "TopLevel");
                    EXPECT_EQ(ast.struct_definitions[0]->modifiers.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->modifiers[0].name, "export");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "switch_target_missing_opening_paren_recovers",
                .source_code = "let x = switch 1 ) {\n    case A -> 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedLeftParenAfterSwitch, .line = 1, .column = 16}
                },
                .verify_ast = ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "switch_target_unclosed_paren_recovers_at_brace",
                .source_code = "let x = switch ( 1 {\n    case A -> 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightParenAfterSwitchTarget, .line = 1, .column = 18}
                },
                .verify_ast = ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "switch_target_top_level_declaration_1",
                .source_code = "let x = switch(let a = 1) {\n    case A -> 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 1, .column = 16}
                },
                .verify_ast = ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "switch_target_top_level_declaration_2",
                .source_code = "let x = switch(enum Test: int { A }) {\n    case A -> 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 1, .column = 16}
                },
                .verify_ast = ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "switch_target_top_level_declaration_3",
                .source_code = "let x = switch(func test() -> int {}) {\n    case A -> 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 1, .column = 16}
                },
                .verify_ast = ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "switch_target_top_level_declaration_4",
                .source_code = "let x = switch(struct Data {}) {\n    case A -> 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 1, .column = 16}
                },
                .verify_ast = ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "unclosed_switch_inside_grouping_escapes_properly",
                .source_code = "let x = (switch(v) { case A -> 1) + 2\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::CaseOrDefaultMissingInSwitchAfterResult, .line = 1, .column = 33},
                    {.code = Err::ExpectedCaseOrDefaultInsideSwitchBody, .line = 1, .column = 33},
                    {.code = Err::ExpectedRightBraceAfterSwitchBody, .line = 1, .column = 37},
                    {.code = Err::ExpectedRightParenAfterExpression, .line = 1, .column = 37}
                },
                .verify_ast = [](const Program &ast) {
                    EXPECT_GE(ast.execution_steps.size(), 1);
                    auto last_stmt = dynamic_cast<const Assignment*>(ast.execution_steps.back().get());
                    ASSERT_NE(last_stmt, nullptr);
                    EXPECT_EQ(last_stmt->targets[0].name, "a");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "return_inside_switch_case_result",
                .source_code = "func test() -> int {\n    let x = switch(v) {\n        case A -> return 1\n    }\n}\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 3, .column = 19}
                },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    auto sw = dynamic_cast<const Assignment*>(ast.function_definitions[0]->body[0].get());
                    ASSERT_NE(sw, nullptr);
                }
            }
        ),
        TestNameGenerator{}
    );
}
