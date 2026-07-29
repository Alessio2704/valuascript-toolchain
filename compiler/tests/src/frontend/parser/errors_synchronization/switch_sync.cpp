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
            "missing_closing_brace_escapes_to_top_level",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "func top_level() -> void {}\n",
            {{Err::ExpectedRightBraceAfterSwitchBody, 2, 15}},
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->name, "top_level");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_struct_without_closing_brace_escapes_to_top_level",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    struct TopLevel { id: int }\n"
            "let c = 1\n",
            {{Err::ExpectedRightBraceAfterSwitchBody, 2, 15}},
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.struct_definitions.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->name, "TopLevel");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_enum_without_closing_brace_escapes_to_top_level",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    enum TopLevel: int { A = 1 }\n"
            "let c = 1\n",
            {{Err::ExpectedRightBraceAfterSwitchBody, 2, 15}},
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.enum_definitions.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->name, "TopLevel");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_statements_between_cases",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    let y = 2\n"
            "    foo()\n"
            "    case B -> 3\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 3, 5}
            },
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "nested_switch_steals_outer_cases_on_missing_brace",
            "let x = switch(v) {\n"
            "    case A -> switch(y) {\n"
            "        case B -> 1\n"
            "    case C -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedRightBraceAfterSwitchBody, 5, 1}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1) << "Outer switch should only have 1 case.";

            auto inner_sw = dynamic_cast<const SwitchExpression*>(sw->cases[0].result.get());
            ASSERT_NE(inner_sw, nullptr);
            EXPECT_EQ(inner_sw->cases.size(), 2) << "Inner switch should have stolen the second case.";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_func_in_closed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    func nested() -> void {}\n"
            "    case B -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 3, 5}
            },
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_struct_in_closed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    struct Nested { id: int }\n"
            "    case B -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 3, 5}
            },
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_directive_in_closed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    #pragma = 1\n"
            "    case B -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 3, 5}
            },
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_func_escapes_unclosed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "func top_level() -> void {}\n",
            {
            {Err::ExpectedRightBraceAfterSwitchBody, 2, 15}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->name, "top_level");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_directive_escapes_unclosed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "#pragma = 1\n",
            {
            {Err::ExpectedRightBraceAfterSwitchBody, 2, 15}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.directives.size(), 1);
            EXPECT_EQ(ast.directives[0]->name, "pragma");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_escaped_declaration_unclosed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "@export struct TopLevel { id: int }\n",
            {
            {Err::ExpectedRightBraceAfterSwitchBody, 2, 15}
            },
            [](const Program &ast) {
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
            "switch_target_missing_opening_paren_recovers",
            "let x = switch 1 ) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedLeftParenAfterSwitch, 1, 16}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_unclosed_paren_recovers_at_brace",
            "let x = switch ( 1 {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedRightParenAfterSwitchTarget, 1, 18}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_top_level_declaration_1",
            "let x = switch(let a = 1) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 16}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_top_level_declaration_2",
            "let x = switch(enum Test: int { A }) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 16}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_top_level_declaration_3",
            "let x = switch(func test() -> int {}) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 16}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_top_level_declaration_4",
            "let x = switch(struct Data {}) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 16}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "unclosed_switch_inside_grouping_escapes_properly",
            "let x = (switch(v) { case A -> 1) + 2\n"
            "let a = 1\n",
            {
            {Err::CaseOrDefaultMissingInSwitchAfterResult, 1, 33},
            {Err::ExpectedCaseOrDefaultInsideSwitchBody, 1, 33},
            {Err::ExpectedRightBraceAfterSwitchBody, 1, 37},
            {Err::ExpectedRightParenAfterExpression, 1, 37}
            },
            [](const Program &ast) {
            EXPECT_GE(ast.execution_steps.size(), 1);
            auto last_stmt = dynamic_cast<const Assignment*>(ast.execution_steps.back().get());
            ASSERT_NE(last_stmt, nullptr);
            EXPECT_EQ(last_stmt->targets[0].name, "a");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "return_inside_switch_case_result",
            "func test() -> int {\n"
            "    let x = switch(v) {\n"
            "        case A -> return 1\n"
            "    }\n"
            "}\n",
            {
            {Err::InvalidExpression, 3, 19}
            },
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto sw = dynamic_cast<const Assignment*>(ast.function_definitions[0]->body[0].get());
            ASSERT_NE(sw, nullptr);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
