#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct TupleHappyParam {
    std::string test_id;
    std::string source_code;
};

class TupleHappyPathTest : public test::AstBaseTest,
                           public testing::WithParamInterface<TupleHappyParam> {
};

TEST_P(TupleHappyPathTest, ParsesSuccessfully) {
    const TupleHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_expression_as_assignment(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        EXPECT_EQ(assignment->targets.size(), 1);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    TupleHappyPathTest,
    testing::Values(
        TupleHappyParam{"tuple", "(1,2,3)"},
        TupleHappyParam{"tuple_1", "(a, 2, c+1)"},
        TupleHappyParam{"tuple_2", "(a, 2, if a then 1 else 2)"}
    ),
    [](const testing::TestParamInfo<TupleHappyParam>& info) {
    return info.param.test_id;
    }
);

struct TupleSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class TupleSadPathTest : public test::AstBaseTest,
                         public testing::WithParamInterface<TupleSadParam> {
};

TEST_P(TupleSadPathTest, ThrowsCorrectSyntaxError) {
    const TupleSadParam &param = GetParam();

    try {
        parse_expression_as_assignment(param.source_code);
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
    TupleSadPathTest,
    testing::Values(
    TupleSadParam{"tuple_missing_second_value_1", "(1, ", ErrorCode::ExpectedRightParenAfterTupleElements},
    TupleSadParam{"tuple_missing_second_value_2", "(a, ", ErrorCode::ExpectedRightParenAfterTupleElements},
    TupleSadParam{"tuple_parenthesis", "(a, b", ErrorCode::ExpectedRightParenAfterTupleElements},
    TupleSadParam{"tuple_trailing_comma_1", "(a, b,)", ErrorCode::TrailingCommaInTuple},
    TupleSadParam{"single_element_tuples_not_allowed", "(1, )", ErrorCode::SingleElementTuplesNotAllowed},
    TupleSadParam{"missing_operator_1", "(a b)", ErrorCode::MissingOperatorInsideGrouping},
    TupleSadParam{"missing_operator_2", "(a, b c)", ErrorCode::MissingCommaOrOperatorBetweenExpressions},
    TupleSadParam{"missing_operator_3", "(a, b (c + d))", ErrorCode::MissingOperatorOrArgumentName},
    TupleSadParam{"missing_operator_4", "(a, b + (c  d))", ErrorCode::MissingOperatorInsideGrouping}
    ),
    [](const testing::TestParamInfo<TupleSadParam>& info) {
    return info.param.test_id;
    }
);
