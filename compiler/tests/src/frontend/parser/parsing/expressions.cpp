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
            ExpressionSadParam{"unclosed_parenthesis", "(1 + 2", E::ExpectedRightParenAfterExpression
            },
            ExpressionSadParam{"unmatched_right_parenthesis", "1 + 2)", E::InvalidExpression},
            ExpressionSadParam{"unclosed_vector_literal", "[1, 2, 3", E::UnmatchedBracketAfterTensorElements},

            ExpressionSadParam{"cond_missing_then", "if a > b 1 else 2", E::MissingThenToken},
            ExpressionSadParam{"cond_missing_else", "if a > b then 1", E::MissingElseToken},
            ExpressionSadParam{"cond_missing_else_value", "if a > b then 1 else", E::InvalidExpression},


            ExpressionSadParam{"missing_operator_1", "a + b c", E::MissingOperator},
            ExpressionSadParam{"missing_operator_2", "a + b (1 + 2)", E::MissingOperatorOrArgumentName},
            ExpressionSadParam{"missing_operator_3", "a + b model.a", E::MissingOperator},
            ExpressionSadParam{"missing_operator_4", "a + b vec[0]", E::MissingOperator},
            ExpressionSadParam{"missing_operator_5", "a + b {}", E::MissingOperator},
            ExpressionSadParam{"missing_operator_6", "a  b[]", E::MissingOperator},
            ExpressionSadParam{"missing_operator_6_a", "a - b[]", E::EmptyBracketAccess},
            ExpressionSadParam{"missing_operator_7", "a + b (1, 2)", E::MissingOperatorOrArgumentName}
        ),
        [](const testing::TestParamInfo<ExpressionSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
