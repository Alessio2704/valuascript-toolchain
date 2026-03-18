#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::shared;
using namespace valuascript::compiler;

struct BracketAccessHappyParam {
    std::string test_id;
    std::string source_code;
};

class BracketAccessHappyPathTest : public test::AstBaseTest,
                            public testing::WithParamInterface<BracketAccessHappyParam> {
};

TEST_P(BracketAccessHappyPathTest, ParsesSuccessfully) {
    const BracketAccessHappyParam &param = GetParam();

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
    BracketAccessHappyPathTest,
    testing::Values(
        BracketAccessHappyParam{"vector_access_simple", "vec[0]"},
        BracketAccessHappyParam{"vector_slice", "vec[:1]"},
        BracketAccessHappyParam{"vector_access_nested", "matrix[0][1]"},
        BracketAccessHappyParam{"vector_access_with_math", "vec[i + 1]"},
        BracketAccessHappyParam{"vector_slice_double", "vec[1:2]"},
        BracketAccessHappyParam{"vector_slice_double_1", "vec[a:b]"},
        BracketAccessHappyParam{"vector_slice_double_2", "vec[a : b - 1]"},
        BracketAccessHappyParam{"vector_slice_both_empty", "vec[:]"},
        BracketAccessHappyParam{"chain_call_then_access", "get_vector()[0]"},
        BracketAccessHappyParam{"chain_access_then_call", "array_of_funcs[0](a: arg)"},
        BracketAccessHappyParam{"chain_deep_mixed", "get_matrix()[0][:1]"}
    ),
    [](const testing::TestParamInfo<BracketAccessHappyParam>& info) {
    return info.param.test_id;
    }
);

struct BracketAccessSadParam {
    std::string test_id;
    std::string source_code;
    ValuascriptErrorCode expected_error;
};

class BracketAccessSadPathTest : public test::AstBaseTest,
                          public testing::WithParamInterface<BracketAccessSadParam> {
};

TEST_P(BracketAccessSadPathTest, ThrowsCorrectSyntaxError) {
    const BracketAccessSadParam &param = GetParam();

    try {
        parse_expression_as_assignment(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    BracketAccessSadPathTest,
    testing::Values(
        BracketAccessSadParam{"unclosed_vector_access", "vec[0", ValuascriptErrorCode::UnmatchedBracketAfterTensorIndex},
        BracketAccessSadParam{"empty_vector_access", "vec[]", ValuascriptErrorCode::EmptyBracketAccess},
        BracketAccessSadParam{"missing_operator_1", "vec[1 2]", ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor},
        BracketAccessSadParam{"missing_operator_2", "vec[1 + 2 3]", ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor},
        BracketAccessSadParam{"missing_operator_3", "vec[1 + (2 3)]", ValuascriptErrorCode::MissingOperatorInsideGrouping},
        BracketAccessSadParam{"missing_operator_4", "vec[1  (2 + 3)]", ValuascriptErrorCode::MissingOperatorOrArgumentName},
        BracketAccessSadParam{"missing_operator_5", "vec[1 + a() b()]", ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor}
    ),
    [](const testing::TestParamInfo<BracketAccessSadParam>& info) {
    return info.param.test_id;
    }
);
