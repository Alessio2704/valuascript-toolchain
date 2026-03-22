#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

using Err = valuascript::compiler::ValuascriptErrorCode;

namespace {
    const StructDefinition *ExpectRecoveredStruct(const Program &ast, const std::string &expected_name) {
        EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive.";

        EXPECT_EQ(ast.struct_definitions.size(), 1);
        const auto *struct_def = ast.struct_definitions.front().get();
        EXPECT_EQ(struct_def->name, expected_name);
        return struct_def;
    }

    void ExpectStructFields(const StructDefinition *struct_def,
                            const std::vector<std::pair<std::string, std::string> > &expected_fields) {
        ASSERT_NE(struct_def, nullptr) << "Struct definition was null!";
        ASSERT_EQ(struct_def->fields.size(), expected_fields.size()) << "Recovered field count mismatch!";

        for (size_t i = 0; i < expected_fields.size(); ++i) {
            EXPECT_EQ(struct_def->fields[i].first, expected_fields[i].first)
                << "Field name mismatch at index " << i;

            ASSERT_NE(struct_def->fields[i].second, nullptr)
                << "Type annotation missing at index " << i;

            EXPECT_EQ(struct_def->fields[i].second->name, expected_fields[i].second)
                << "Type name mismatch at index " << i;
        }
    }

    auto ExpectNoStructs() {
        return [](const Program &ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 0);
            ASSERT_EQ(ast.execution_steps.size(), 1);
        };
    }

    auto ExpectStruct(std::string name, std::vector<std::pair<std::string, std::string> > fields = {}) {
        return [name = std::move(name), fields = std::move(fields)](const Program &ast) {
            auto s = ExpectRecoveredStruct(ast, name);
            ExpectStructFields(s, fields);
        };
    }
}

class StructParserSynchronizationTest : public ParserErrorsSynchronizationBase {
};

TEST_P(StructParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
    run_parser_and_check_errors(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    ParserExhaustiveStressTests,
    StructParserSynchronizationTest,
    ::testing::Values(
        ParserErrorsSynchronizationTestCase{
        "no_name_struct_empty_ast",
        "struct { id: int }\n"
        "let a = 1\n",
        { {Err::ExpectedStructName, 1, 8} },
        ExpectNoStructs()
        },
        ParserErrorsSynchronizationTestCase{
        "no_left_brace_struct_empty_ast",
        "struct Test id: int }\n"
        "let a = 1\n",
        { {Err::ExpectedBraceInStructDefinition, 1, 13} },
        ExpectNoStructs()
        },
        ParserErrorsSynchronizationTestCase{
        "no_right_brace_struct_empty_ast",
        "struct Test { id: int \n"
        "let a = 1\n",
        { {Err::ExpectedRightBraceAfterStructBody, 1, 22} },
        ExpectNoStructs()
        },
        ParserErrorsSynchronizationTestCase{
        "no_colon_empty_struct_in_ast",
        "struct Test { id int } \n"
        "let a = 1\n",
        { {Err::ExpectedColonAfterStructFieldName, 1, 18} },
        ExpectStruct("Test")
        },
        ParserErrorsSynchronizationTestCase{
        "no_field_name_empty_struct_in_ast",
        "struct Test {  : int } \n"
        "let a = 1\n",
        { {Err::ExpectedStructFieldName, 1, 16} },
        ExpectStruct("Test")
        },
        ParserErrorsSynchronizationTestCase{
        "no_commas_all_fields_struct_in_ast",
        "struct Test { host: string port: int speed: int mode: string }\n"
        "let a = 1\n",
        {
        {Err::ExpectedCommaSeparatorInStruct, 1, 28},
        {Err::ExpectedCommaSeparatorInStruct, 1, 38},
        {Err::ExpectedCommaSeparatorInStruct, 1, 49},
        },
        ExpectStruct("Test", {
            {"host", "string"},
            {"port", "int"},
            {"speed", "int"},
            {"mode", "string"}
            })
        },
        ParserErrorsSynchronizationTestCase{
        "no_type_all_other_fields_struct_in_ast",
        "struct Test { host: string port: int speed: int mode: }\n"
        "let a = 1\n",
        {
        {Err::ExpectedCommaSeparatorInStruct, 1, 28},
        {Err::ExpectedCommaSeparatorInStruct, 1, 38},
        {Err::ExpectedCommaSeparatorInStruct, 1, 49},
        {Err::MissingTypeAnnotation, 1, 55},
        },
        ExpectStruct("Test", {
            {"host", "string"},
            {"port", "int"},
            {"speed", "int"}
            })
        },
        ParserErrorsSynchronizationTestCase{
        "no_colon_all_other_fields_struct_in_ast",
        "struct Test { host: string port: int speed int mode: string }\n"
        "let a = 1\n",
        {
        {Err::ExpectedCommaSeparatorInStruct, 1, 28},
        {Err::ExpectedCommaSeparatorInStruct, 1, 38},
        {Err::ExpectedColonAfterStructFieldName, 1, 44}
        },
        ExpectStruct("Test", {
            {"host", "string"},
            {"port", "int"}
            })
        },
        ParserErrorsSynchronizationTestCase{
        "no_colon_plus_second_last_comma_all_other_fields_struct_in_ast",
        "struct Test { host: string port: int speed int, mode: string }\n"
        "let a = 1\n",
        {
        {Err::ExpectedCommaSeparatorInStruct, 1, 28},
        {Err::ExpectedCommaSeparatorInStruct, 1, 38},
        {Err::ExpectedColonAfterStructFieldName, 1, 44}
        },
        ExpectStruct("Test", {
            {"host", "string"},
            {"port", "int"},
            {"mode", "string"}
            })
        }
    ),
    [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
    return info.param.test_name;
    }
);
