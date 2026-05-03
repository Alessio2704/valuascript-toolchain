#include <gtest/gtest.h>
#include "../errors_synchronization/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct FunctionCallSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class FunctionCallSadPathTest : public AstBaseTest,
                                    public testing::WithParamInterface<FunctionCallSadParam>
    {
    };

    TEST_P(FunctionCallSadPathTest, ThrowsCorrectSyntaxError)
    {
        const FunctionCallSadParam& param = GetParam();

        try
        {
            parse_code("let result = " + param.source_code);
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
        FunctionCallSadPathTest,
        testing::Values(
            FunctionCallSadParam{"func_missing_argument", "test(1)", ValuascriptErrorCode::MissingOperatorOrArgumentName
            },
            FunctionCallSadParam{"func_missing_colon", "test(a 1)", ValuascriptErrorCode::MissingColonAfterArgument},
            FunctionCallSadParam{"func_missing_argument_value", "test(a: )", ValuascriptErrorCode::InvalidExpression},
            FunctionCallSadParam{"func_trailing_comma", "test(a: 1, )", ValuascriptErrorCode::
            TrailingCommaInFunctionCall},
            FunctionCallSadParam{"unclosed_call", "test(a: 1, b: 2", ValuascriptErrorCode::
            ExpectedRightParenAfterArguments},
            FunctionCallSadParam{"missing_comma_separator", "test(a: 1 b: 2)", ValuascriptErrorCode::
            MissingCommaSeparatorForArgumentsInFunctionCall}
        ),
        [](const testing::TestParamInfo<FunctionCallSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
