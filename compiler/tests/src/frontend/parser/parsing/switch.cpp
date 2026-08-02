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
            SwitchSadParam{.test_name = "missing_left_paren", .source_code = "let a = switch res) { case UP -> 1 }", .expected_error = E::
            ExpectedLeftParenAfterSwitch},
            SwitchSadParam{.test_name = "missing_right_paren", .source_code = "let a = switch (res { case UP -> 1 }", .expected_error = E::ExpectedRightParenAfterSwitchTarget},
            SwitchSadParam{.test_name = "missing_left_brace", .source_code = "let a = switch (res) case UP -> 1 }", .expected_error = E::
            ExpectedLeftBraceBeforeSwitchBody},
            SwitchSadParam{.test_name = "missing_right_brace", .source_code = "let a = switch (res) { case UP -> 1", .expected_error = E::
            ExpectedRightBraceAfterSwitchBody},
            SwitchSadParam{.test_name = "expression_as_case", .source_code = "let a = switch (res) { case a + b -> 10 }", .expected_error = E::ExpectedRightArrowAfterSwitchCaseIdentifier},
            SwitchSadParam{.test_name = "assignment_in_body", .source_code = "let a = switch (res) { let b = 2 }", .expected_error = E::TopLevelDeclarationNotAllowedHere}

        ),
        [](const testing::TestParamInfo<SwitchSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
