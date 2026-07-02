#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct EnumSadParam {
        std::string test_name;
        std::string source_code;
        E expected_error;
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
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserInvalidEnumDefinitions,
        EnumSadTest,
        testing::Values(
            EnumSadParam{"missing_colon", "enum Option string { a }", E::ExpectedColonAfterEnumName},
            EnumSadParam{"missing_left_brace", "enum Option: string a }", E::
            ExpectedLeftBraceBeforeEnumBody},
            EnumSadParam{"missing_right_brace", "enum Option: string { a, b ", E::
            ExpectedRightBraceAfterEnumBody},
            EnumSadParam{"invalid_value_expression", "enum Option: string { a = let }", E::
            ReservedKeywordAsIdentifier}
        ),
        [](const testing::TestParamInfo<EnumSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
