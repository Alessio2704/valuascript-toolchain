#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::shared;
using namespace valuascript::compiler;

struct EnumHappyParam {
    std::string test_id;
    std::string source_code;
};

class EnumHappyTest : public test::AstBaseTest,
                      public testing::WithParamInterface<EnumHappyParam> {
};

TEST_P(EnumHappyTest, ParsesSuccessfully) {
    auto param = GetParam();
    std::shared_ptr<Program> ast;

    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser choked on valid enum syntax: " << param.test_id << "\nCode: " << param.source_code;

    ASSERT_NE(ast, nullptr);
    EXPECT_EQ(ast->enum_definitions.size(), 1) << "Expected exactly 1 enum definition to be parsed.";
}

INSTANTIATE_TEST_SUITE_P(
    ValidEnumDefinitions,
    EnumHappyTest,
    testing::Values(
        EnumHappyParam{"implicit_values", "enum OptionType: string { call, put }"},
        EnumHappyParam{"explicit_strings", "enum Direction: string { UP = \"up\", DOWN = \"down\" }"},
        EnumHappyParam{"explicit_vector", "enum Test: vector<int> { ONE = [1,2,3,4], TWO = [5,6,7,8] }"},
        EnumHappyParam{"explicit_matrix", "enum Test: vector<int> { ONE = [[1,2],[3,4]], TWO = [[5,6],[7,8]] }"},
        EnumHappyParam{"explicit_numbers", "enum Status: integer { ACTIVE = 1, PENDING = 2 }"},
        EnumHappyParam{"mixed_values", "enum Codes: integer { START = 100, CONTINUE, END = 999 }"},
        EnumHappyParam{"expression_values", "enum Math: decimal { PI = 3.14, TAU = 3.14 * 2.0 }"},
        EnumHappyParam{"empty_enum", "enum Phantom: string {}"},
        EnumHappyParam{"custom_type", "enum Complex: Result<decimal, string> { OK, ERR }"},
        EnumHappyParam{"trailing_comma", "enum Colors: string { red, blue, }"}
    ),
    [](const testing::TestParamInfo<EnumHappyParam>& info) {
    return info.param.test_id;
    }
);

struct EnumSadParam {
    std::string test_id;
    std::string source_code;
    ValuascriptErrorCode expected_error;
};

class EnumSadTest : public test::AstBaseTest,
                    public testing::WithParamInterface<EnumSadParam> {
};

TEST_P(EnumSadTest, ThrowsCorrectSyntaxError) {
    auto param = GetParam();

    try {
        parse_code(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    InvalidEnumDefinitions,
    EnumSadTest,
    testing::Values(
        EnumSadParam{"missing_name", "enum : string { a }", ValuascriptErrorCode::ExpectedEnumName},
        EnumSadParam{"missing_colon", "enum Option string { a }", ValuascriptErrorCode::ExpectedColonAfterEnumName},
        EnumSadParam{"missing_type", "enum Option: { a }", ValuascriptErrorCode::MissingTypeAnnotation},
        EnumSadParam{"missing_left_brace", "enum Option: string a }", ValuascriptErrorCode::ExpectedLeftBraceBeforeEnumBody},
        EnumSadParam{"invalid_case_name_number", "enum Option: string { 1 = \"a\" }", ValuascriptErrorCode::ExpectedEnumCaseName},
        EnumSadParam{"invalid_case_name_string", "enum Option: string { \"call\" }", ValuascriptErrorCode::ExpectedEnumCaseName},
        EnumSadParam{"missing_right_brace", "enum Option: string { a, b ", ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody},
        EnumSadParam{"invalid_value_expression", "enum Option: string { a = let }", ValuascriptErrorCode::InvalidExpression},
        EnumSadParam{"missing_comma", "enum Scenario: string { LOW BASE, HIGH }", ValuascriptErrorCode::
        ExpectedCommaSeparatorInEnum},
        EnumSadParam{"keyword_as_case_name", "enum Bad: string { if = \"a\" }", ValuascriptErrorCode::ExpectedEnumCaseName},
        EnumSadParam{"missing_operator_1", "enum Test : int { A = a b, B = 2 }", ValuascriptErrorCode::MissingOperator},
        EnumSadParam{"missing_operator_2", "enum Test : int { A = (a b), B = 2 }", ValuascriptErrorCode::MissingOperatorInsideGrouping},
        EnumSadParam{"missing_operator_3", "enum Test : int { A = 1 (a + b), B = 2 }", ValuascriptErrorCode::MissingOperatorOrArgumentName}
    ),
    [](const testing::TestParamInfo<EnumSadParam>& info) {
    return info.param.test_id;
    }
);
