#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/parser/ast.h"
#include "stages/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserExpressionTestBase : public testing::Test {
protected:
    std::shared_ptr<Program> parse_expression_as_assignment(const std::string &expression) {
        std::string code = "let result = " + expression;

        LexerStage lexer;
        std::vector<CompilerStageArtifact> lexer_history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        };
        auto lexer_result = lexer.run(lexer_history);

        ParserStage parser;
        std::vector<CompilerStageArtifact> parser_history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        };
        auto parser_result = parser.run(parser_history);

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }
};

struct ExpressionHappyParam {
    std::string test_id;
    std::string expression_code;
};

class ExpressionHappyPathTest : public ParserExpressionTestBase,
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

        ExpressionHappyParam{"call_no_args", "my_func()"},
        ExpressionHappyParam{"call_multiple_args", "my_func(a: 1, b: 2, c: 3)"},
        ExpressionHappyParam{"call_nested", "outer_func(a: inner_func(a: x), b: y)"},
        ExpressionHappyParam{"call_with_math_args", "calc(one: a + b, second: c * d)"},

        ExpressionHappyParam{"vector_access_simple", "vec[0]"},
        ExpressionHappyParam{"vector_slice", "vec[:1]"},
        ExpressionHappyParam{"vector_access_nested", "matrix[0][1]"},
        ExpressionHappyParam{"vector_access_with_math", "vec[i + 1]"},
        ExpressionHappyParam{"vector_slice_double", "vec[1:2]"},
        ExpressionHappyParam{"vector_slice_double_1", "vec[a:b]"},
        ExpressionHappyParam{"vector_slice_double_2", "vec[a : b - 1]"},
        ExpressionHappyParam{"vector_slice_both_empty", "vec[:]"},

        ExpressionHappyParam{"chain_call_then_access", "get_vector()[0]"},
        ExpressionHappyParam{"chain_access_then_call", "array_of_funcs[0](a: arg)"},
        ExpressionHappyParam{"chain_deep_mixed", "get_matrix()[0][:1]"},
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

class ExpressionSadPathTest : public ParserExpressionTestBase,
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

        ExpressionSadParam{"unclosed_parenthesis", "(1 + 2", ErrorCode::UnmatchedParenthesis},
        ExpressionSadParam{"unmatched_right_parenthesis", "1 + 2)", ErrorCode::UnexpectedToken},
        ExpressionSadParam{"unclosed_vector_literal", "[1, 2, 3", ErrorCode::UnmatchedBracket},

        ExpressionSadParam{"unclosed_vector_access", "vec[0", ErrorCode::UnmatchedBracket},
        ExpressionSadParam{"empty_vector_access", "vec[]", ErrorCode::EmptyVectorAccess},

        ExpressionSadParam{"cond_missing_then", "if a > b 1 else 2", ErrorCode::MissingThenToken},
        ExpressionSadParam{"cond_missing_else", "if a > b then 1", ErrorCode::MissingElseToken},
        ExpressionSadParam{"cond_missing_else_value", "if a > b then 1 else", ErrorCode::InvalidExpression},
        ExpressionSadParam{"func_missing_argument", "some_func(1)", ErrorCode::MissingArgumentName},
        ExpressionSadParam{"func_missing_colon", "some_func(a 1)", ErrorCode::MissingColonAfterArgument},
        ExpressionSadParam{"func_missing_argument_value", "some_func(a: )", ErrorCode::InvalidExpression},
        ExpressionSadParam{"func_missing_argument_after_comma", "some_func(a: 1, )", ErrorCode::
        MissingArgumentName},
        ExpressionSadParam{"unclosed_call", "my_func(a: 1, b: 2", ErrorCode::UnmatchedParenthesis}
    ),
    [](const testing::TestParamInfo<ExpressionSadParam>& info) {
    return info.param.test_id;
    }
);
