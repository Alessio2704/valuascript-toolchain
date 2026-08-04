#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const StructDefinition *ExpectRecoveredStruct(const Program &ast, const std::string &expected_name) {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive.";

            EXPECT_EQ(ast.struct_definitions.size(), 1);
            const auto *struct_def = ast.struct_definitions.front().get();
            EXPECT_EQ(struct_def->name, expected_name);
            return struct_def;
        }

        void ExpectStructFields(const StructDefinition *struct_def,
                                const std::vector<std::pair<std::string, std::optional<std::string> > > &
                                expected_fields) {
            ASSERT_NE(struct_def, nullptr) << "Struct definition was null!";
            ASSERT_EQ(struct_def->fields.size(), expected_fields.size()) << "Recovered field count mismatch!";

            for (size_t i = 0; i < expected_fields.size(); ++i) {
                EXPECT_EQ(struct_def->fields[i].name, expected_fields[i].first)
                     << "Field name mismatch at index " << i;

                if (expected_fields[i].second.has_value()) {
                    ASSERT_NE(struct_def->fields[i].type, nullptr)
                        << "Type annotation missing at index " << i;

                    EXPECT_EQ(struct_def->fields[i].type->name, expected_fields[i].second)
                         << "Type name mismatch at index " << i;
                } else {
                    ASSERT_EQ(struct_def->fields[i].type, nullptr)
                        << "Type annotation not missing at index " << i;
                }
            }
        }

        auto ExpectNoStructs() {
            return [](const Program &ast) {
                ASSERT_EQ(ast.struct_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }

        auto ExpectStruct(std::string name,
                          std::vector<std::pair<std::string, std::optional<std::string> > > fields = {}) {
            return [n = std::move(name), f = std::move(fields)](const Program &ast) {
                auto s = ExpectRecoveredStruct(ast, n);
                ExpectStructFields(s, f);
            };
        }
    }

    class StructParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(StructParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        StructStressTest,
        StructParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "name_reserved_keyword_full_ast",
                .source_code = "struct true { host: string, port: int, speed: int }\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 8}
                },
                .verify_ast = ExpectStruct("true", {
                    {"host", "string"},
                    {"port", "int"},
                    {"speed", "int"},
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "no_left_brace_struct_empty_ast",
                .source_code = "struct Test id: int }\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedBraceInStructDefinition, .line = 1, .column = 13} },
                .verify_ast = ExpectNoStructs()
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "reserved_keyword_1",
                .source_code = "struct Test { host: string, port: int, let: int }\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 40}
                },
                .verify_ast = ExpectStruct("Test", {
                    {"host", "string"},
                    {"port", "int"},
                    {"let", "int"},
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "reserved_keyword_2",
                .source_code = "struct Test { host: string port: int let: int }\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedCommaSeparatorInStruct, .line = 1, .column = 28},
                    {.code = Err::ExpectedCommaSeparatorInStruct, .line = 1, .column = 38},
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 38}
                },
                .verify_ast = ExpectStruct("Test", {
                    {"host", "string"},
                    {"port", "int"},
                    {"let", "int"},
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "reserved_keyword_3",
                .source_code = "struct Test { case: string, if: int, let: int }\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 15},
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 29},
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 38}
                },
                .verify_ast = ExpectStruct("Test", {
                    {"case", "string"},
                    {"if", "int"},
                    {"let", "int"},
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "reserved_keyword_4",
                .source_code = "struct Test { host: string, port: int, true int }\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ReservedKeywordAsIdentifier, .line = 1, .column = 40},
                    {.code = Err::ExpectedColonAfterStructFieldName, .line = 1, .column = 45},
                },
                .verify_ast = ExpectStruct("Test", {
                    {"host", "string"},
                    {"port", "int"},
                    {"<error>", std::nullopt},
                })
            }
        ),
        TestNameGenerator{}
    );
}
