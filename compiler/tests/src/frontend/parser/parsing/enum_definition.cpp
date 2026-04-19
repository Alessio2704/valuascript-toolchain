#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct EnumSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class EnumSadTest : public AstBaseTest,
                        public testing::WithParamInterface<EnumSadParam> {
    };

    TEST_P(EnumSadTest, ThrowsCorrectSyntaxError) {
        auto param = GetParam();

        try {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
                << "Category mismatch on test: " << param.test_name;
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserInvalidEnumDefinitions,
        EnumSadTest,
        testing::Values(
            EnumSadParam{"missing_name", "enum : string { a }", ValuascriptErrorCode::ExpectedEnumName},
            EnumSadParam{"missing_colon", "enum Option string { a }", ValuascriptErrorCode::ExpectedColonAfterEnumName},
            EnumSadParam{"missing_type", "enum Option: { a }", ValuascriptErrorCode::MissingTypeAnnotation},
            EnumSadParam{"missing_left_brace", "enum Option: string a }", ValuascriptErrorCode::
            ExpectedLeftBraceBeforeEnumBody},
            EnumSadParam{"invalid_case_name_number", "enum Option: string { 1 = \"a\" }", ValuascriptErrorCode::
            ExpectedEnumCaseName},
            EnumSadParam{"invalid_case_name_string", "enum Option: string { \"call\" }", ValuascriptErrorCode::
            ExpectedEnumCaseName},
            EnumSadParam{"missing_right_brace", "enum Option: string { a, b ", ValuascriptErrorCode::
            ExpectedRightBraceAfterEnumBody},
            EnumSadParam{"invalid_value_expression", "enum Option: string { a = let }", ValuascriptErrorCode::
            ReservedKeywordAsIdentifier},
            EnumSadParam{"missing_comma", "enum Scenario: string { LOW BASE, HIGH }", ValuascriptErrorCode::
            ExpectedCommaSeparatorInEnum},
            EnumSadParam{"keyword_as_case_name", "enum Bad: string { if = \"a\" }", ValuascriptErrorCode::
            ReservedKeywordAsIdentifier},
            EnumSadParam{"missing_operator_1", "enum Test : int { A = a b, B = 2 }", ValuascriptErrorCode::
            MissingOperator},
            EnumSadParam{"missing_operator_2", "enum Test : int { A = (a b), B = 2 }", ValuascriptErrorCode::
            MissingOperatorInsideGrouping},
            EnumSadParam{"missing_operator_3", "enum Test : int { A = 1 (a + b), B = 2 }", ValuascriptErrorCode::
            MissingOperatorOrArgumentName}
        ),
        [](const testing::TestParamInfo<EnumSadParam>& info) {
        return info.param.test_name;
        }
    );
}
