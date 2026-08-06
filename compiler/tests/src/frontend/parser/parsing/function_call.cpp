#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct FunctionCallSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
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
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        FunctionCallSadPathTest,
        testing::Values(
            FunctionCallSadParam{.test_name = "func_missing_argument", .source_code = "test(1)", .expected_error = E::MissingOperator}
        ),
        TestNameGenerator{}
    );
}
