#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const EnumDefinition *ExpectRecoveredEnum(const Program &ast, const std::string &expected_name) {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive.";
            EXPECT_EQ(ast.enum_definitions.size(), 1);
            const auto *enum_def = ast.enum_definitions.front().get();
            EXPECT_EQ(enum_def->name, expected_name);
            return enum_def;
        }

        struct ExpectedEnumCase {
            std::string name;
            std::optional<std::string> expected_number_value;

            ExpectedEnumCase(const char *n) : name(n), expected_number_value(std::nullopt) {
            }

            ExpectedEnumCase(const char *n, const char *v) : name(n), expected_number_value(std::string(v)) {
            }
        };

        void ExpectEnumCases(const EnumDefinition *enum_def,
                             const std::string &expected_type,
                             const std::vector<ExpectedEnumCase> &expected_cases) {
            ASSERT_NE(enum_def, nullptr) << "Enum definition was null!";
            ASSERT_NE(enum_def->underlying_type, nullptr) << "Type annotation missing!";
            EXPECT_EQ(enum_def->underlying_type->name, expected_type) << "Underlying type mismatch!";
            ASSERT_EQ(enum_def->cases.size(), expected_cases.size()) << "Recovered case count mismatch!";

            for (size_t i = 0; i < expected_cases.size(); ++i) {
                EXPECT_EQ(enum_def->cases[i].first, expected_cases[i].name)
                     << "Case name mismatch at index " << i;

                if (expected_cases[i].expected_number_value.has_value()) {
                    ASSERT_NE(enum_def->cases[i].second, nullptr)
                         << "Expected an assigned value for case '" << expected_cases[i].name << "' but got nullptr";
                    auto *num_lit = dynamic_cast<NumberLiteral *>(enum_def->cases[i].second.get());
                    ASSERT_NE(num_lit, nullptr)
                         << "Expected a NumberLiteral for case '" << expected_cases[i].name << "'";

                    EXPECT_EQ(num_lit->value, expected_cases[i].expected_number_value.value())
                         << "Assigned value mismatch for case '" << expected_cases[i].name << "'";
                } else {
                    EXPECT_EQ(enum_def->cases[i].second, nullptr)
                         << "Expected NO assigned value for case '" << expected_cases[i].name << "' but found one";
                }
            }
        }

        auto ExpectEnum(std::string name, std::string type, std::vector<ExpectedEnumCase> cases = {}) {
            return [name = std::move(name), type = std::move(type), cases = std::move(cases)](const Program &ast) {
                auto e = ExpectRecoveredEnum(ast, name);
                ExpectEnumCases(e, type, cases);
            };
        }

        auto ExpectNoEnums() {
            return [](const Program &ast) {
                ASSERT_EQ(ast.enum_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }
    }

    class EnumParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(EnumParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserExhaustiveStressTests,
        EnumParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "no_name_enum_empty_ast",
            "enum : int { A }\n"
            "let a = 1\n",
            { {Err::ExpectedEnumName, 1, 6} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "name_reserved_keyword_full_ast",
            "enum true: int { A = 1, B = 2 }\n"
            "let a = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 6} },
            ExpectEnum("true", "int", {{"A", "1"}, {"B", "2"}})
            },
            ParserErrorsSynchronizationTestCase{
            "no_colon_enum_empty_ast",
            "enum Test int { A }\n"
            "let a = 1\n",
            { {Err::ExpectedColonAfterEnumName, 1, 11} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "no_type_enum_empty_ast",
            "enum Test : { A }\n"
            "let a = 1\n",
            { {Err::MissingTypeAnnotation, 1, 13} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "wrong_generic_type_enum_empty_ast_1",
            "enum Test : vector<int { A }\n"
            "let a = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 24} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "wrong_generic_type_enum_empty_ast_2",
            "enum Test : vector<int, > { A }\n"
            "let a = 1\n",
            {{Err::TrailingCommaInGenericArgument, 1, 23}},
            [] (const Program & ast) {
            auto enum_def = ExpectRecoveredEnum(ast, "Test");
            EXPECT_EQ(enum_def->underlying_type.get()->name, "vector");
            EXPECT_EQ(enum_def->underlying_type.get()->generic_args.size(), 1);
            EXPECT_EQ(enum_def->underlying_type.get()->generic_args[0].get()->name, "int");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "wrong_generic_type_enum_empty_ast_3",
            "enum Test : vector<*> { A }\n"
            "let a = 1\n",
            {
            {Err::MissingTypeAnnotation, 1, 20},
            {Err::EmptyGenericTypeAnnotation, 1, 21}
            },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "wrong_generic_type_enum_empty_ast_4",
            "enum Test : vector<int, decimal, > { A }\n"
            "let a = 1\n",
            {{Err::TrailingCommaInGenericArgument, 1, 32}},
            [] (const Program & ast) {
            auto enum_def = ExpectRecoveredEnum(ast, "Test");
            EXPECT_EQ(enum_def->underlying_type.get()->name, "vector");
            EXPECT_EQ(enum_def->underlying_type.get()->generic_args.size(), 2);
            EXPECT_EQ(enum_def->underlying_type.get()->generic_args[0].get()->name, "int");
            EXPECT_EQ(enum_def->underlying_type.get()->generic_args[1].get()->name, "decimal");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "no_left_brace_enum_empty_ast",
            "enum Test : int A }\n"
            "let a = 1\n",
            { {Err::ExpectedLeftBraceBeforeEnumBody, 1, 17} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "no_right_brace_enum_empty_ast",
            "enum Test : int { A \n"
            "let a = 1\n",
            { {Err::ExpectedRightBraceAfterEnumBody, 1, 20} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "no_commas_all_cases_enum_in_ast",
            "enum Test : int { Red Green Blue }\n"
            "let a = 1\n",
            {
            {Err::ExpectedCommaSeparatorInEnum, 1, 23},
            {Err::ExpectedCommaSeparatorInEnum, 1, 29}
            },
            ExpectEnum("Test", "int", {"Red", "Green", "Blue"})
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_tokens_discarded_recovers_valid_cases",
            "enum Test : int { Red, +-*/, Blue }\n"
            "let a = 1\n",
            { {Err::ExpectedEnumCaseName, 1, 24} },
            ExpectEnum("Test", "int", {"Red", "Blue"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_case_name_recovers_valid_cases",
            "enum Test : int { Red, , Blue }\n"
            "let a = 1\n",
            { {Err::ExpectedEnumCaseName, 1, 24} },
            ExpectEnum("Test", "int", {"Red", "Blue"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_value_assignment_discards_case_and_recovers",
            "enum Test : int { Red =, Blue }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 24} },
            ExpectEnum("Test", "int", {"Blue"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_value_assignment_discards_case_and_recovers_multiple",
            "enum Test : int { Red =, Blue =, Green, Yellow, Black, White = }\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 24},
            {Err::InvalidExpression, 1, 32},
            {Err::InvalidExpression, 1, 64},
            },
            ExpectEnum("Test", "int", {"Green", "Yellow", "Black"})
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_expression_after_assign_discards_case_and_recovers",
            "enum Test : int { A = *10, B = 2 }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 23} },
            ExpectEnum("Test", "int", {{"B", "2"}})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_comma_after_valid_assignment_recovers_both_cases",
            "enum Test : int { A = 1 B = 2 }\n"
            "let a = 1\n",
            { {Err::ExpectedCommaSeparatorInEnum, 1, 25} },
            ExpectEnum("Test", "int", {{"A", "1"}, {"B", "2"}})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_expression_at_end_of_enum_discards_last_case_only",
            "enum Test : int { A = 1, B = }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 30} },
            ExpectEnum("Test", "int", {{"A", "1"}})
            },
            ParserErrorsSynchronizationTestCase{
            "complex_expression_with_inner_error_discards_case",
            "enum Test : int { A = (1 + *), B = 1 }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 28} },
            [](const Program& ast) {
            auto recovered_enum = ExpectRecoveredEnum(ast, "Test");
            EXPECT_EQ(recovered_enum->cases.size(), 2);
            EXPECT_EQ(recovered_enum->cases[0].first, "A");
            auto first_case_value = dynamic_cast<GroupingExpression*>(recovered_enum->cases[0].second.get());
            ASSERT_NE(first_case_value, nullptr);
            EXPECT_EQ(recovered_enum->cases[1].first, "B");
            auto second_case_value = dynamic_cast<NumberLiteral*>(recovered_enum->cases[1].second.get());
            ASSERT_NE(second_case_value, nullptr);
            },
            },
            ParserErrorsSynchronizationTestCase{
            "valid_assignments_are_captured_in_ast",
            "enum Test : int { A = 1, B = 2 }\n"
            "let a = 1\n",
            {},
            ExpectEnum("Test", "int", { {"A", "1"}, {"B", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "mixed_assignments_are_captured_in_ast",
            "enum Test : int { A, B = 2, C }\n"
            "let a = 1\n",
            {},
            ExpectEnum("Test", "int", { {"A"}, {"B", "2"}, {"C"} })
            },
            ParserErrorsSynchronizationTestCase{
            "missing_comma_recovers_both_cases_and_their_values",
            "enum Test : int { A = 10 B = 20 }\n"
            "let a = 1\n",
            { {Err::ExpectedCommaSeparatorInEnum, 1, 26} },
            ExpectEnum("Test", "int", { {"A", "10"}, {"B", "20"} })
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_discards_corrupt_assignment_but_saves_valid_ones",
            "enum Test : int { A = 1, B = +-*/, C = 3 }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 32} },
            ExpectEnum("Test", "int", { {"A", "1"}, {"C", "3"} })
            },
            ParserErrorsSynchronizationTestCase{
            "missing_expression_discards_case_and_saves_valid_ones",
            "enum Test : int { A = 1, B = , C = 3 }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 30} },
            ExpectEnum("Test", "int", { {"A", "1"}, {"C", "3"} })
            },
            ParserErrorsSynchronizationTestCase{
            "missing_expression_at_end_discards_last_case_only",
            "enum Test : int { A = 1, B = }\n"
            "let a = 1\n",
            { {Err::InvalidExpression, 1, 30} },
            ExpectEnum("Test", "int", { {"A", "1"} })
            },
            ParserErrorsSynchronizationTestCase{
            "missing_expression_and_brace_should_invalidate_all_enum_definition",
            "enum Test : int { A = 1, B = \n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 29},
            {Err::ExpectedRightBraceAfterEnumBody, 1, 29}
            },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "complex_expression_1",
            "enum Test : int { A = if a 1 else 2, B = 2 } \n"
            "let a = 1\n",
            {
            {Err::MissingThenToken, 1, 28},
            },
            ExpectEnum("Test", "int", {{"B", "2"}})
            },
            ParserErrorsSynchronizationTestCase{
            "complex_expression_2",
            "enum Test : int { A = if a 1 else 2, B = if a 1 else 2 }\n"
            "let a = 1\n",
            {
            {Err::MissingThenToken, 1, 28},
            {Err::MissingThenToken, 1, 47},
            },
            ExpectEnum("Test", "int", {})
            },
            ParserErrorsSynchronizationTestCase{
            "complex_expression_3",
            "enum Test : int { A = a b, B = 2 }\n"
            "let a = 1\n",
            {
            {Err::MissingOperator, 1, 25},
            },
            ExpectEnum("Test", "int", {{"B", "2"}})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_1",
            "enum Test : int { let, true, if }\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 19},
            {Err::ReservedKeywordAsIdentifier, 1, 24},
            {Err::ReservedKeywordAsIdentifier, 1, 30},
            },
            ExpectEnum("Test", "int", {"let", "true", "if"})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_2",
            "enum Test : int { let = 1, true = 2, if = 3 }\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 19},
            {Err::ReservedKeywordAsIdentifier, 1, 28},
            {Err::ReservedKeywordAsIdentifier, 1, 38},
            },
            ExpectEnum("Test", "int", {{"let", "1"}, {"true", "2"}, {"if", "3"}})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_3",
            "enum Test : int { let true if }\n"
            "let a = 1\n",
            {
            {Err::ExpectedEnumCaseName, 1, 19},
            {Err::ExpectedRightBraceAfterEnumBody, 1, 19},
            {Err::InvalidIdentifier, 1, 23},
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.enum_definitions.size(), 0);
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_4",
            "enum Test : int { let = 1, true = , if = 3 }\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 19},
            {Err::ReservedKeywordAsIdentifier, 1, 28},
            {Err::InvalidExpression, 1, 35},
            {Err::ReservedKeywordAsIdentifier, 1, 37},
            },
            ExpectEnum("Test", "int", {{"let", "1"}, {"if", "3"}})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_char",
            "enum Test : int { # }\n"
            "let a = 1\n",
            {
            {Err::ExpectedEnumCaseName, 1, 19},
            {Err::ExpectedRightBraceAfterEnumBody, 1, 19},
            {Err::MissingDirectiveName, 1, 21},
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.enum_definitions.size(), 0);
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
