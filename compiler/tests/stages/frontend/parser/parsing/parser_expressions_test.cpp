#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct ExpressionHappyParam {
    std::string test_id;
    std::string expression_code;
};

class ExpressionHappyPathTest : public test::AstBaseTest,
                                public testing::WithParamInterface<ExpressionHappyParam> {
};

TEST_P(ExpressionHappyPathTest, ParsesSuccessfully) {
    const ExpressionHappyParam &param = GetParam();

    EXPECT_NO_THROW({
        parse_expression_as_assignment(param.expression_code);
        }) << "Parser threw an exception on valid expression test: " << param.test_id;
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
        ExpressionHappyParam{"access_on_literal", "1[0]"}
    ),
    [](const testing::TestParamInfo<ExpressionHappyParam>& info) {
    return info.param.test_id;
    }
);

struct ExpressionSadParam {
    std::string test_id;
    std::string expression_code;
    ErrorCode expected_error;
};

class ExpressionSadPathTest : public test::AstBaseTest,
                              public testing::WithParamInterface<ExpressionSadParam> {
};

TEST_P(ExpressionSadPathTest, ThrowsCorrectSyntaxError) {
    const ExpressionSadParam &param = GetParam();

    try {
        parse_expression_as_assignment(param.expression_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    ExpressionSadPathTest,
    testing::Values(
        ExpressionSadParam{"missing_rhs_addition", "1 + ", ErrorCode::InvalidExpression},
        ExpressionSadParam{"missing_rhs_comparison", "a > ", ErrorCode::InvalidExpression},
        ExpressionSadParam{"missing_unary_operand", "not ", ErrorCode::InvalidExpression},

        ExpressionSadParam{"unclosed_parenthesis", "(1 + 2", ErrorCode::ExpectedRightParenAfterExpression},
        ExpressionSadParam{"unmatched_right_parenthesis", "1 + 2)", ErrorCode::UnexpectedTopLevelToken},
        ExpressionSadParam{"unclosed_vector_literal", "[1, 2, 3", ErrorCode::UnmatchedBracketAfterVectorElements},

        ExpressionSadParam{"cond_missing_then", "if a > b 1 else 2", ErrorCode::MissingThenToken},
        ExpressionSadParam{"cond_missing_else", "if a > b then 1", ErrorCode::MissingElseToken},
        ExpressionSadParam{"cond_missing_else_value", "if a > b then 1 else", ErrorCode::InvalidExpression}
    ),
    [](const testing::TestParamInfo<ExpressionSadParam>& info) {
    return info.param.test_id;
    }
);
