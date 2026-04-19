#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct SwitchSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class SwitchSadPathTest : public AstBaseTest,
                              public testing::WithParamInterface<SwitchSadParam>
    {
    };

    TEST_P(SwitchSadPathTest, ThrowsCorrectSyntaxError)
    {
        const SwitchSadParam& param = GetParam();

        try
        {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        }
        catch (const ValuaScriptException& e)
        {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
                << "Category mismatch on test: " << param.test_name;
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        SwitchSadPathTest,
        testing::Values(
            SwitchSadParam{"missing_left_paren", "let a = switch res) { case UP -> 1 }", ValuascriptErrorCode::
            ExpectedLeftParenAfterSwitch},
            SwitchSadParam{"missing_right_paren", "let a = switch (res { case UP -> 1 }",
            ValuascriptErrorCode::ExpectedRightParenAfterSwitchTarget},
            SwitchSadParam{"missing_left_brace", "let a = switch (res) case UP -> 1 }", ValuascriptErrorCode::
            ExpectedLeftBraceBeforeSwitchBody},
            SwitchSadParam{"missing_right_brace", "let a = switch (res) { case UP -> 1", ValuascriptErrorCode::
            ExpectedRightBraceAfterSwitchBody},
            SwitchSadParam{"number_as_case", "let a = switch (res) { case 1 -> 10 }",
            ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase},
            SwitchSadParam{"string_as_case", "let a = switch (res) { case \"UP\" -> 10 }",
            ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase},
            SwitchSadParam{"expression_as_case", "let a = switch (res) { case a + b -> 10 }",
            ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier},
            SwitchSadParam{"missing_arrow_case", "let a = switch (res) { case UP 10 }",
            ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier},
            SwitchSadParam{"missing_arrow_default", "let a = switch (res) { default 10 }", ValuascriptErrorCode::
            ExpectedRightArrowAfterSwitchCaseIdentifier}
            ,
            SwitchSadParam{"duplicate_default", "let a = switch (res) { default -> 1 default -> 2 }",
            ValuascriptErrorCode::MultipleDefaultCasesInSwitch},
            SwitchSadParam{"assignment_in_body", "let a = switch (res) { let b = 2 }",
            ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere},
            SwitchSadParam{"missing_operator_1", "let a = switch (res) { case UP -> 1 2 }", ValuascriptErrorCode::
            MissingOperatorInSwitchCaseResult},
            SwitchSadParam{"missing_operator_2", "let a = switch (res) { case UP -> 1 (2 + 3) }", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            SwitchSadParam{"missing_operator_3", "let a = switch (res) { case UP -> 1 + a() (2 + 3) }",
            ValuascriptErrorCode::MissingOperatorOrArgumentName},
            SwitchSadParam{"missing_operator_4", "let a = switch (res) { case UP -> 1 + a() b() }", ValuascriptErrorCode
            ::MissingOperatorInSwitchCaseResult}

        ),
        [](const testing::TestParamInfo<SwitchSadParam>& info) {
        return info.param.test_name;
        }
    );
}
