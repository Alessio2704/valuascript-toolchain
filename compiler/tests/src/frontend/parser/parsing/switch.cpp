#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct SwitchSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
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
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        SwitchSadPathTest,
        testing::Values(
            SwitchSadParam{"missing_left_paren", "let a = switch res) { case UP -> 1 }", E::
            ExpectedLeftParenAfterSwitch},
            SwitchSadParam{"missing_right_paren", "let a = switch (res { case UP -> 1 }",
            E::ExpectedRightParenAfterSwitchTarget},
            SwitchSadParam{"missing_left_brace", "let a = switch (res) case UP -> 1 }", E::
            ExpectedLeftBraceBeforeSwitchBody},
            SwitchSadParam{"missing_right_brace", "let a = switch (res) { case UP -> 1", E::
            ExpectedRightBraceAfterSwitchBody},
            SwitchSadParam{"expression_as_case", "let a = switch (res) { case a + b -> 10 }",
            E::ExpectedRightArrowAfterSwitchCaseIdentifier},
            SwitchSadParam{"assignment_in_body", "let a = switch (res) { let b = 2 }",
            E::TopLevelDeclarationNotAllowedHere}

        ),
        [](const testing::TestParamInfo<SwitchSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
