#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct FunctionSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class FunctionSadPathTest : public AstBaseTest,
                                public testing::WithParamInterface<FunctionSadParam>
    {
    };

    TEST_P(FunctionSadPathTest, ThrowsCorrectSyntaxError)
    {
        const FunctionSadParam& param = GetParam();

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
        FunctionSadPathTest,
        testing::Values(
            FunctionSadParam{"missing_func_name", "func () -> scalar {}", E::MissingFunctionName},
            FunctionSadParam{"missing_left_paren", "func test) -> scalar {}", E::
            ExpectedLeftParenAfterFunctionName},
            FunctionSadParam{"missing_right_paren", "func test(a: scalar -> scalar {}", E::
            ExpectedRightParenAfterParameters},
            FunctionSadParam{"missing_arrow", "func test() scalar {}", E::MissingArrowInFunction},
            FunctionSadParam{"malformed_arrow_1", "func test() scalar - {}", E::
            MissingArrowInFunction},
            FunctionSadParam{"malformed_arrow_2", "func test() scalar > {}", E::
            MissingArrowInFunction},
            FunctionSadParam{"missing_left_brace", "func test() -> scalar }", E::
            ExpectedLeftBraceBeforeFunctionBody},
            FunctionSadParam{"missing_right_brace", "func test() -> scalar { return 1", E::
            ExpectedRightBraceAfterFunctionBody},
            FunctionSadParam{"missing_param_name", "func test(: scalar) -> scalar {}", E::
            MissingParameterName},
            FunctionSadParam{"missing_colon", "func test(a scalar) -> scalar {}", E::
            MissingColonAfterParameter},
            FunctionSadParam{"missing_param_type", "func test(a: ) -> scalar {}", E::
            MissingTypeAnnotation},
            FunctionSadParam{"unclosed_generic", "func test(a: vector<scalar) -> scalar {}", E::
            UnmatchedBracketAfterGenericArgs},
            FunctionSadParam{"missing_return_type", "func test() -> {}", E::
            MissingTypeAnnotationAfterArrow},
            FunctionSadParam{"unclosed_tuple_return", "func test() -> (scalar, bool {}", E::
            UnmatchedParenthesisInTuple},
            FunctionSadParam{"invalid_statement_in_body", "func test() -> scalar { 1 + 1 }", E::
            InvalidStandaloneStatement},
            FunctionSadParam{"missing_comma_in_params", "func test(a: scalar b: decimal) -> scalar { return 1 + 1 }",
            E::ExpectedCommaSeparatorInParameterList},
            FunctionSadParam{"missing_comma_return",
            "func test(a: scalar, b: decimal) -> scalar decimal { return 1 + 1 }", E::
            ExpectedCommaSeparatorInReturnTypeList},
            FunctionSadParam{"top_level_declaration_in_func_1",
            "func test(a: s) -> s { return 1 \n let a = b()\n enum Test: s {} }\n", E::
            TopLevelDeclarationNotAllowedHere},
            FunctionSadParam{"top_level_declaration_in_func_2",
            "func test(a: s) -> s { return 1 \n let a = b()\n struct Test {} }\n", E::
            TopLevelDeclarationNotAllowedHere},
            FunctionSadParam{"top_level_declaration_in_func_3",
            "func test(a: s) -> s { return 1 \n let a = b()\n #dir\n }", E::
            TopLevelDeclarationNotAllowedHere},
            FunctionSadParam{"top_level_declaration_in_func_4",
            "func test(a: s) -> s { return 1 \n let a = b()\n func other() -> scalar {}\n }", E::
            TopLevelDeclarationNotAllowedHere},
            FunctionSadParam{"single_param_default", "func test(a: scalar = ) -> scalar {}", E::
            MissingDefaultParameterValue},
            FunctionSadParam{"multi_param_default", "func test(a: scalar = 1, b: boolean) -> scalar {}",
            E::NonDefaultParameterAfterDefault}
        ),
        [](const testing::TestParamInfo<FunctionSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
