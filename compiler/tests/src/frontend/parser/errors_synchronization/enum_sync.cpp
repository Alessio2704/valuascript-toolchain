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
            std::optional<std::string> expected_number_value = std::nullopt;
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
                .test_name = "no_colon_enum_empty_ast",
                .source_code = "enum Test int { A }\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedColonAfterEnumName, .line = 1, .column = 11} },
                .verify_ast = ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "no_left_brace_enum_empty_ast",
                .source_code = "enum Test : int A }\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedLeftBraceBeforeEnumBody, .line = 1, .column = 17} },
                .verify_ast = ExpectNoEnums()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_expression_and_brace",
                .source_code = "enum Test : int { A = 1, B = \nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 1, .column = 28},
                    {.code = Err::ExpectedRightBraceAfterEnumBody, .line = 1, .column = 28}
                },
                .verify_ast = ExpectEnum("Test", "int", {
                    {.name = "A", .expected_number_value = "1"},
                    {.name = "B"}
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "reserved_char_2",
                .source_code = "enum Test : int { / }\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedEnumCaseName, .line = 1, .column = 19},
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.enum_definitions.size(), 1);
                    EXPECT_EQ(ast.enum_definitions[0]->cases.size(), 1);
                    EXPECT_EQ(ast.enum_definitions[0]->cases[0].name, "<error>");
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                }
            }
        ),
        TestNameGenerator{}
    );
}
