#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct ExpressionSadParam
    {
        std::string test_name;
        std::string expression_code;
        ValuascriptErrorCode expected_error;
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
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        ExpressionSadPathTest,
        testing::Values(
            ExpressionSadParam{"missing_rhs_addition", "1 + ", ValuascriptErrorCode::InvalidExpression},
            ExpressionSadParam{"missing_rhs_comparison", "a > ", ValuascriptErrorCode::InvalidExpression},
            ExpressionSadParam{"missing_unary_operand", "not ", ValuascriptErrorCode::InvalidExpression},

            ExpressionSadParam{"unclosed_parenthesis", "(1 + 2", ValuascriptErrorCode::ExpectedRightParenAfterExpression
            },
            ExpressionSadParam{"unmatched_right_parenthesis", "1 + 2)", ValuascriptErrorCode::InvalidExpression},
            ExpressionSadParam{"unclosed_vector_literal", "[1, 2, 3", ValuascriptErrorCode::
            UnmatchedBracketAfterTensorElements},

            ExpressionSadParam{"cond_missing_then", "if a > b 1 else 2", ValuascriptErrorCode::MissingThenToken},
            ExpressionSadParam{"cond_missing_else", "if a > b then 1", ValuascriptErrorCode::MissingElseToken},
            ExpressionSadParam{"cond_missing_else_value", "if a > b then 1 else", ValuascriptErrorCode::
            InvalidExpression},


            ExpressionSadParam{"missing_operator_1", "a + b c", ValuascriptErrorCode::MissingOperator},
            ExpressionSadParam{"missing_operator_2", "a + b (1 + 2)", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            ExpressionSadParam{"missing_operator_3", "a + b model.a", ValuascriptErrorCode::MissingOperator},
            ExpressionSadParam{"missing_operator_4", "a + b vec[0]", ValuascriptErrorCode::MissingOperator},
            ExpressionSadParam{"missing_operator_5", "a + b {}", ValuascriptErrorCode::MissingOperator},
            ExpressionSadParam{"missing_operator_6", "a  b[]", ValuascriptErrorCode::MissingOperator},
            ExpressionSadParam{"missing_operator_6_a", "a - b[]", ValuascriptErrorCode::EmptyBracketAccess},
            ExpressionSadParam{"missing_operator_7", "a + b (1, 2)", ValuascriptErrorCode::
            MissingOperatorOrArgumentName}
        ),
        [](const testing::TestParamInfo<ExpressionSadParam>& info) {
        return info.param.test_name;
        }
    );
}
