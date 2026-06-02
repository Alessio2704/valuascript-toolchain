#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const EnumDefinition* ExpectRecoveredEnum(const Program& ast, const std::string& expected_name)
        {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive.";
            EXPECT_EQ(ast.enum_definitions.size(), 1);
            const auto* enum_def = ast.enum_definitions.front().get();
            EXPECT_EQ(enum_def->name, expected_name);
            return enum_def;
        }

        struct ExpectedEnumCase
        {
            std::string name;
            std::optional<std::string> expected_number_value;

            ExpectedEnumCase(const char* n) : name(n), expected_number_value(std::nullopt)
            {
            }

            ExpectedEnumCase(const char* n, const char* v) : name(n), expected_number_value(std::string(v))
            {
            }
        };

        void ExpectEnumCases(const EnumDefinition* enum_def,
                             const std::optional<std::string>& expected_type,
                             const std::vector<ExpectedEnumCase>& expected_cases)
        {
            ASSERT_NE(enum_def, nullptr) << "Enum definition was null!";

            if (expected_type.has_value())
            {
                ASSERT_NE(enum_def->underlying_type, nullptr) << "Type annotation missing!";
                EXPECT_EQ(enum_def->underlying_type->name, expected_type.value()) << "Underlying type mismatch!";
            }
            else
            {
                ASSERT_EQ(enum_def->underlying_type, nullptr) << "Type annotation is not missing!";
            }

            ASSERT_EQ(enum_def->cases.size(), expected_cases.size()) << "Recovered case count mismatch!";

            for (size_t i = 0; i < expected_cases.size(); ++i)
            {
                EXPECT_EQ(enum_def->cases[i].name, expected_cases[i].name)
                     << "Case name mismatch at index " << i;

                if (expected_cases[i].expected_number_value.has_value())
                {
                    ASSERT_NE(enum_def->cases[i].value, nullptr)
                         << "Expected an assigned value for case '" << expected_cases[i].name << "' but got nullptr";
                    auto* num_lit = dynamic_cast<NumberLiteral*>(enum_def->cases[i].value.get());
                    ASSERT_NE(num_lit, nullptr)
                         << "Expected a NumberLiteral for case '" << expected_cases[i].name << "'";

                    EXPECT_EQ(num_lit->value, expected_cases[i].expected_number_value.value())
                         << "Assigned value mismatch for case '" << expected_cases[i].name << "'";
                }
                else
                {
                    EXPECT_EQ(enum_def->cases[i].value, nullptr)
                         << "Expected NO assigned value for case '" << expected_cases[i].name << "' but found one";
                }
            }
        }

        auto ExpectEnum(std::string name, std::optional<std::string> type, std::vector<ExpectedEnumCase> cases = {})
        {
            return [n = std::move(name), t = std::move(type), c = std::move(cases)](const Program& ast)
            {
                auto e = ExpectRecoveredEnum(ast, n);
                ExpectEnumCases(e, t, c);
            };
        }

        auto ExpectNoEnums()
        {
            return [](const Program& ast)
            {
                ASSERT_EQ(ast.enum_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }
    }

    class EnumParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(EnumParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        EnumStressTest,
        EnumParserSynchronizationTest,
        ::testing::Values(
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
            "wrong_generic_type_enum_ast_1",
            "enum Test : vector<int { A }\n"
            "let a = 1\n",
            { {Err::UnmatchedBracketAfterGenericArgs, 1, 24} },
            ExpectEnum("Test", "vector", {"A"})
            },
            ParserErrorsSynchronizationTestCase{
            "no_left_brace_enum_empty_ast",
            "enum Test : int A }\n"
            "let a = 1\n",
            { {Err::ExpectedLeftBraceBeforeEnumBody, 1, 17} },
            ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_expression_and_brace",
            "enum Test : int { A = 1, B = \n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 28},
            {Err::ExpectedRightBraceAfterEnumBody, 1, 29}
            },
            ExpectEnum("Test", "int", {{"A", "1"}, "B"})
            },
            ParserErrorsSynchronizationTestCase{
            "complex_expression_3",
            "enum Test : int { A = a b, B = 2 }\n"
            "let a = 1\n",
            {
            {Err::MissingOperator, 1, 25},
            },
            [](const Program& ast) {
            auto& enum_def = ast.enum_definitions[0];
            ASSERT_EQ(enum_def->cases.size(), 2);
            auto val_1 = dynamic_cast<IdentifierAccess*>(enum_def->cases[0].value.get());
            ASSERT_NE(val_1, nullptr);
            ASSERT_EQ(val_1->name, "a");
            auto val_2 = dynamic_cast<NumberLiteral*>(enum_def->cases[1].value.get());
            ASSERT_NE(val_2, nullptr);
            ASSERT_EQ(val_2->value, "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_char_1",
            "enum Test : int { # }\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 19},
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.enum_definitions.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->cases.size(), 1);
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_char_2",
            "enum Test : int { / }\n"
            "let a = 1\n",
            {
            {Err::ExpectedEnumCaseName, 1, 19},
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.enum_definitions.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->cases.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->cases[0].name, "<error>");
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
