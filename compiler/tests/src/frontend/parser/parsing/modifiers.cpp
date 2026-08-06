#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct ModifierSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class ModifierSadPathTest : public AstBaseTest,
                                public testing::WithParamInterface<ModifierSadParam>
    {
    };

    TEST_P(ModifierSadPathTest, ThrowsCorrectSyntaxError)
    {
        auto param = GetParam();
        try
        {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        }
        catch (const ValuaScriptException& e)
        {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax);
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ModifierErrorTests,
        ModifierSadPathTest,
        testing::Values(
            ModifierSadParam{.test_name = "DoubleAtSign", .source_code = "@@export let x = 1", .expected_error = E::ExpectedModifierName},
            ModifierSadParam{.test_name = "StructFieldModifierOnClosingBrace", .source_code = "struct S { @ }", .expected_error = E::
            ExpectedModifierName}
        ),
        TestNameGenerator{}
    );
}
