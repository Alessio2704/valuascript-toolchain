#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct ExpressionHappyParam {
        std::string test_name;
        std::string expression_code;
    };

    class ExpressionHappyPathTest : public AstBaseTest,
                                    public testing::WithParamInterface<ExpressionHappyParam> {
    };

    TEST_P(ExpressionHappyPathTest, ParsesSuccessfully) {
        const ExpressionHappyParam &param = GetParam();

        EXPECT_NO_THROW({
            parse_expression_as_assignment(param.expression_code);
            }) << "Parser threw an exception on valid expression test: " << param.test_name;
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        ExpressionHappyPathTest,
        testing::Values(
            ExpressionHappyParam{"math_standard_precedence", "1 + 2 * 3^4"},
            ExpressionHappyParam{"math_parentheses_override", "(1 + 2) * 3"},
            ExpressionHappyParam{"math_deep_nesting", "(((1 + 2) * 3) / 4) - 5"},
            ExpressionHappyParam{"unary_chaining", "- - 5"},

            ExpressionHappyParam{"chain_function_returning_function", "factory_func()(a: arg)"},

            ExpressionHappyParam{"bool_standard", "a and b or c"},
            ExpressionHappyParam{"bool_grouped", "(a and b) or (c and not d)"},
            ExpressionHappyParam{"bool_with_comparisons", "(x > 0) and (y <= 10)"},

            ExpressionHappyParam{"cond_simple", "if a > b then a else b"},
            ExpressionHappyParam{"cond_nested", "if a then 1 else if b then 2 else 3"},
            ExpressionHappyParam{"cond_with_math", "if (x + y) > 10 then (x * 2) else (y / 2)"},

            ExpressionHappyParam{"call_on_literal", "1()"},
            ExpressionHappyParam{"access_on_literal", "1[0]"},

            ExpressionHappyParam{"conditional_expression", "if true then 10 else 4"},
            ExpressionHappyParam{"or_expr", "x or y"},
            ExpressionHappyParam{"and_expr", "x and y"},
            ExpressionHappyParam{"not_expr", "not x"},
            ExpressionHappyParam{"eq_expr", "x == y"},
            ExpressionHappyParam{"neq_expr", "x != y"},
            ExpressionHappyParam{"gt_expr", "x > y"},
            ExpressionHappyParam{"lt_expr", "x < y"},
            ExpressionHappyParam{"gte_expr", "x >= y"},
            ExpressionHappyParam{"lte_expr", "x <= y"},
            ExpressionHappyParam{"pow_expr", "x^y"}
        ),
        [](const testing::TestParamInfo<ExpressionHappyParam>& info) {
        return info.param.test_name;
        }
    );

    struct ExpressionSadParam {
        std::string test_name;
        std::string expression_code;
        ValuascriptErrorCode expected_error;
    };

    class ExpressionSadPathTest : public AstBaseTest,
                                  public testing::WithParamInterface<ExpressionSadParam> {
    };

    TEST_P(ExpressionSadPathTest, ThrowsCorrectSyntaxError) {
        const ExpressionSadParam &param = GetParam();

        try {
            parse_expression_as_assignment(param.expression_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
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
            MissingOperatorOrArgumentName},
            ExpressionSadParam{"chaining_not_allowed_for_comparison_1", "a > b > c", ValuascriptErrorCode::
            ChainingNotAllowedForComparisonOperations},
            ExpressionSadParam{"chaining_not_allowed_for_comparison_2", "10 <= 5 != false", ValuascriptErrorCode
            ::
            ChainingNotAllowedForComparisonOperations}
        ),
        [](const testing::TestParamInfo<ExpressionSadParam>& info) {
        return info.param.test_name;
        }
    );
}
