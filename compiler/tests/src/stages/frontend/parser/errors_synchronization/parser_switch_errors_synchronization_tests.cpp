#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const SwitchExpression *ExpectRecoveredSwitch(const Program &ast) {
            EXPECT_GE(ast.execution_steps.size(), 1) << "Expected an assignment at the top level.";
            if (ast.execution_steps.empty()) return nullptr;

            auto assign = dynamic_cast<const Assignment *>(ast.execution_steps[0].get());
            if (!assign) return nullptr;

            auto switch_expr = dynamic_cast<const SwitchExpression *>(assign->value.get());
            return switch_expr;
        }

        auto ExpectSwitchCases(size_t expected_cases, bool expected_has_default) {
            return [expected_cases, expected_has_default](const Program &ast) {
                auto sw = ExpectRecoveredSwitch(ast);
                ASSERT_NE(sw, nullptr) << "Switch expression not found in the first statement!";
                EXPECT_EQ(sw->cases.size(), expected_cases) << "Switch case count mismatch!";

                if (expected_has_default) {
                    EXPECT_NE(sw->default_case, nullptr) << "Expected default case, but got nullptr!";
                } else {
                    EXPECT_EQ(sw->default_case, nullptr) << "Expected no default case, but got one!";
                }
            };
        }
    }

    class SwitchParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(SwitchParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserExhaustiveStressTests,
        SwitchParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "garbage_token_recovers_to_next_case",
            "let x = switch(v) {\n"
            "    garbage\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::ExpectedCaseOrDefaultInsideSwitchBody, 2, 5}},
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "broken_case_result_recovers_to_next_case",
            "let x = switch(v) {\n"
            "    case A -> 1 + *\n"
            "    case B -> 2\n"
            "}\n"
            "let a = 1\n",
            {{Err::InvalidExpression, 2, 19}},
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_arrow_recovers_to_default",
            "let x = switch(v) {\n"
            "    case A 1\n"
            "    default -> 2\n"
            "}\n"
            "let a = 1\n",
            {{Err::ExpectedRightArrowAfterSwitchCaseIdentifier, 2, 12}},
            ExpectSwitchCases(1, true)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_closing_brace_escapes_to_top_level",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "func top_level() -> void {}\n",
            {{Err::ExpectedRightBraceAfterSwitchBody, 2, 16}},
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->name, "top_level");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_defaults_recovers",
            "let x = switch(v) {\n"
            "    default -> 1\n"
            "    default -> 2\n"
            "}\n"
            "let a = 1\n",
            {{Err::MultipleDefaultCasesInSwitch, 3, 5}},
            ExpectSwitchCases(0, true)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_comma_in_case_identifiers",
            "let x = switch(v) {\n"
            "    case A B -> 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::ExpectedCommaBetweenCaseIdentifiers, 2, 12}},
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_arrow_right_before_closing_brace",
            "let x = switch(v) {\n"
            "    case A -> \n"
            "}\n"
            "let a = 1\n",
            {{Err::InvalidExpression, 2, 12}},
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "nested_struct_without_closing_brace_escapes_to_top_level",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    struct TopLevel { id: int }\n"
            "let c = 1\n",
            {{Err::ExpectedRightBraceAfterSwitchBody, 2, 16}},
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
            {{Err::ExpectedRightBraceAfterSwitchBody, 2, 16}},
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);

            ASSERT_EQ(ast.enum_definitions.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->name, "TopLevel");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "interleaved_valid_and_invalid_switch_cases",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "    garbage\n"
            "    case B -> 2\n"
            "    case C -> 1 + *\n"
            "    default -> 3\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedCaseOrDefaultInsideSwitchBody, 3, 5},
            {Err::InvalidExpression, 5, 19}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 3);

            EXPECT_EQ(sw->cases[0].first[0], "A");
            EXPECT_EQ(sw->cases[1].first[0], "B");
            EXPECT_EQ(sw->cases[2].first[0], "C");

            EXPECT_NE(sw->default_case, nullptr);
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
            "missing_comma_and_garbage_in_case_identifiers",
            "let x = switch(v) {\n"
            "    case A B, C -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedCommaBetweenCaseIdentifiers, 2, 12}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1);
            EXPECT_EQ(sw->cases[0].first.size(), 3);
            EXPECT_EQ(sw->cases[0].first[0], "A");
            EXPECT_EQ(sw->cases[0].first[1], "B");
            EXPECT_EQ(sw->cases[0].first[2], "C");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_switch_recovers_inner_and_outer_errors",
            "let x = switch(v) {\n"
            "    case A -> switch(y) {\n"
            "        garbage\n"
            "        case B -> 1\n"
            "    }\n"
            "    case C -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedCaseOrDefaultInsideSwitchBody, 3, 9}
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
            {Err::ExpectedRightBraceAfterSwitchBody, 5, 2}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 1) << "Outer switch should only have 1 case.";

            auto inner_sw = dynamic_cast<const SwitchExpression*>(sw->cases[0].second.get());
            ASSERT_NE(inner_sw, nullptr);
            EXPECT_EQ(inner_sw->cases.size(), 2) << "Inner switch should have stolen the second case.";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "malformed_case_and_default_declarations",
            "let x = switch(v) {\n"
            "    case -> 1\n"
            "    default A -> 2\n"
            "    case C -> \n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedEnumCaseNameAfterCase, 2, 10},
            {Err::ExpectedRightArrowAfterSwitchCaseIdentifier, 3, 13},
            {Err::InvalidExpression, 4, 12}
            },
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_missing_parentheses_aborts_assignment",
            "let x = switch v {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedLeftParenAfterSwitch, 1, 16}
            },
            [](const Program &ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "empty_switch_body_with_garbage",
            "let x = switch(v) {\n"
            "    + - * /\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedCaseOrDefaultInsideSwitchBody, 2, 5}
            },
            ExpectSwitchCases(0, false)
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_operator_swallows_case_keyword",
            "let x = switch(v) {\n"
            "    case A -> 1 +\n"
            "    case B -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 2, 17},
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases.size(), 2);
            ASSERT_EQ(sw->cases[0].second.get(), nullptr);
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
            "modifier_before_case_in_closed_switch",
            "let x = switch(v) {\n"
            "    @modifier case A -> 1\n"
            "    case B -> 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 2, 5}
            },
            ExpectSwitchCases(2, false)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_func_escapes_unclosed_switch",
            "let x = switch(v) {\n"
            "    case A -> 1\n"
            "func top_level() -> void {}\n",
            {
            {Err::ExpectedRightBraceAfterSwitchBody, 2, 16}
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
            {Err::ExpectedRightBraceAfterSwitchBody, 2, 16}
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
            {Err::ExpectedRightBraceAfterSwitchBody, 2, 16}
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
            "switch_target_broken_expression_recovers",
            "let x = switch(1 + *) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 20}
            },
            ExpectSwitchCases(1, false)
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
            {Err::ExpectedRightParenAfterSwitchTarget, 1, 20}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_completely_empty_parens",
            "let x = switch() {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 16}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "switch_target_garbage_between_parens",
            "let x = switch( . ) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 17}
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
            "switch_target_dangling_operator_swallows_brace",
            "let x = switch(1 + ) {\n"
            "    case A -> 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 20}
            },
            ExpectSwitchCases(1, false)
            },
            ParserErrorsSynchronizationTestCase{
            "broken_switch_as_operand_preserves_outer_expression",
            "let x = 10 + switch(v) { case A -> } + 20\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 36}
            },
            [](const Program &ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<const Assignment*>(ast.execution_steps[0].get());

            auto outer_plus = dynamic_cast<const BinaryExpression*>(assign->value.get());
            ASSERT_NE(outer_plus, nullptr);
            EXPECT_EQ(outer_plus->op, TokenType::Plus);

            auto left_plus = dynamic_cast<const BinaryExpression*>(outer_plus->left.get());
            ASSERT_NE(left_plus, nullptr);
            auto switch_expr = dynamic_cast<const SwitchExpression*>(left_plus->right.get());
            ASSERT_NE(switch_expr, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "unclosed_switch_inside_grouping_escapes_properly",
            "let x = (switch(v) { case A -> 1) + 2\n"
            "let a = 1\n",
            {
            {Err::CaseOrDefaultMissingInSwitchAfterResult, 1, 33},
            {Err::ExpectedCaseOrDefaultInsideSwitchBody, 1, 33},
            {Err::ExpectedRightBraceAfterSwitchBody, 1, 38},
            {Err::ExpectedRightParenAfterExpression, 1, 38}
            },
            [](const Program &ast) {
            EXPECT_GE(ast.execution_steps.size(), 1);
            auto last_stmt = dynamic_cast<const Assignment*>(ast.execution_steps.back().get());
            ASSERT_NE(last_stmt, nullptr);
            EXPECT_EQ(last_stmt->targets[0].first, "a");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "empty_slots_in_case_comma_list",
            "let x = switch(v) {\n"
            "    case A, , B -> 1\n"
            "}\n",
            {
            {Err::ExpectedEnumCaseNameAfterCase, 2, 13}
            },
            [](const Program &ast) {
            auto sw = ExpectRecoveredSwitch(ast);
            ASSERT_NE(sw, nullptr);
            EXPECT_EQ(sw->cases[0].first.size(), 2);
            EXPECT_EQ(sw->cases[0].first[0], "A");
            EXPECT_EQ(sw->cases[0].first[1], "B");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "empty_switch_body_is_valid_but_useless",
            "let x = switch(v) {}\n"
            "let a = 1\n",
            {},
            ExpectSwitchCases(0, false)
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
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
