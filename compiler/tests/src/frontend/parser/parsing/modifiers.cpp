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
            ModifierSadParam{"ModifierOnStandaloneExpression", "@export 10 * 5", E::
            ModifiersAttachedToInvalidDeclaration},
            ModifierSadParam{"ModifierOnReassignment", "let x = 1\n@export x = 2", E::
            ModifiersAttachedToInvalidDeclaration},
            ModifierSadParam{"MissingModifierName", "@ let x = 1", E::ExpectedModifierName},
            ModifierSadParam{"UnclosedParenthesis", "@bind(ui: \"slider\" let x = 1", E::
            UnmatchedParenthesisAfterModifierArgs},
            ModifierSadParam{"DoubleAtSign", "@@export let x = 1", E::ExpectedModifierName},
            ModifierSadParam{"missing_comma_in_param", "@export(a: 1 b: 2) let x = 1", E::
            MissingCommaSeparatorForArgumentsInModifier},
            ModifierSadParam{"missing_operator_1", "@export(a: 1 2) let x = 1", E::MissingOperator},
            ModifierSadParam{"missing_operator_2", "@export(a: 1 + 2 3) let x = 1", E::
            MissingOperator},
            ModifierSadParam{"missing_operator_3", "@export(a: 1 + (2 3)) let x = 1", E::
            MissingOperator},
            ModifierSadParam{"missing_operator_4", "@export(a: 1 (2 + 3)) let x = 1", E::
            MissingOperator},
            ModifierSadParam{"missing_operator_5", "@export(a: 1 a() + b()) let x = 1", E::
            MissingOperator},
            ModifierSadParam{"StructFieldModifierOnClosingBrace", "struct S { @ }", E::
            ExpectedModifierName}
        ),
        [](const testing::TestParamInfo<ModifierSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
