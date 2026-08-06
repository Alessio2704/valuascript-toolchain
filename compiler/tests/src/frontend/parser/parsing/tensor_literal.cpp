#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct TensorLiteralSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class TensorLiteralSadPathTest : public AstBaseTest,
                                     public testing::WithParamInterface<TensorLiteralSadParam>
    {
    };

    TEST_P(TensorLiteralSadPathTest, ThrowsCorrectSyntaxError)
    {
        const TensorLiteralSadParam& param = GetParam();

        try
        {
            parse_expression_as_assignment(param.source_code);
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
        TensorLiteralSadPathTest,
        testing::Values(
            TensorLiteralSadParam{.test_name = "mising_operator_1", .source_code = "[1,2,4] [1,2,3]", .expected_error = E::UnexpectedCommaInBracketAccess},
            TensorLiteralSadParam{.test_name = "mising_operator_2", .source_code = "[1, [2,4]] [[1,2,3], [1,2]]", .expected_error = E::UnexpectedCommaInBracketAccess}
        ),
        TestNameGenerator{}
    );
}
