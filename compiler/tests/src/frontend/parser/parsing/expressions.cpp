#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct ExpressionSadParam
    {
        std::string test_name;
        std::string expression_code;
        E expected_error;
    };

    class ExpressionSadPathTest : public AstBaseTest,
                                  public testing::WithParamInterface<ExpressionSadParam>
    {
    };

    TEST_P(ExpressionSadPathTest, ThrowsCorrectSyntaxError)
    {
        const ExpressionSadParam& param = GetParam();

        try
        {
            parse_expression_as_assignment(param.expression_code);
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
        ExpressionSadPathTest,
        testing::Values(
            ExpressionSadParam{.test_name = "unclosed_parenthesis", .expression_code = "(1 + 2", .expected_error = E::ExpectedRightParenAfterExpression},
            ExpressionSadParam{.test_name = "unmatched_right_parenthesis", .expression_code = "1 + 2)", .expected_error = E::InvalidExpression},
            ExpressionSadParam{.test_name = "unclosed_vector_literal", .expression_code = "[1, 2, 3", .expected_error = E::UnmatchedBracketAfterTensorElements}
        ),
        [](const testing::TestParamInfo<ExpressionSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
