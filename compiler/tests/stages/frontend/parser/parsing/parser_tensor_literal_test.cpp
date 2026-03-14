#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct TensorLiteralHappyParam {
    std::string test_id;
    std::string source_code;
};

class TensorLiteralHappyPathTest : public test::AstBaseTest,
                            public testing::WithParamInterface<TensorLiteralHappyParam> {
};

TEST_P(TensorLiteralHappyPathTest, ParsesSuccessfully) {
    const TensorLiteralHappyParam &param = GetParam();

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
    TensorLiteralHappyPathTest,
    testing::Values(
    TensorLiteralHappyParam{"vector_literal", "[1,2,3]"},
    TensorLiteralHappyParam{"matrix_literal", "[[1,2], [3, 4]]"},
    TensorLiteralHappyParam{"vector_literal_identifiers", "[a, b, c]"},
    TensorLiteralHappyParam{"vector_literal_complex", "[(a, b, c), d, if e then f else g]"},
    TensorLiteralHappyParam{"vector_literal_complex_2", "[(a, b, c()), d(a: 1), if e then f else g()]"},
    TensorLiteralHappyParam{"vector_literal_complex_3", "[(a, b, c()), d(a: 1), switch (b) { default -> 10 } ]"}
    ),
    [](const testing::TestParamInfo<TensorLiteralHappyParam>& info) {
    return info.param.test_id;
    }
);

struct TensorLiteralSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class TensorLiteralSadPathTest : public test::AstBaseTest,
                          public testing::WithParamInterface<TensorLiteralSadParam> {
};

TEST_P(TensorLiteralSadPathTest, ThrowsCorrectSyntaxError) {
    const TensorLiteralSadParam &param = GetParam();

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
    TensorLiteralSadPathTest,
    testing::Values(
    TensorLiteralSadParam{"vector_literal_unclosed", "[1,2,3", ErrorCode::UnmatchedBracket},
    TensorLiteralSadParam{"matrix_literal_unclosed", "[[1,2], [3, 4]", ErrorCode::UnmatchedBracket}
    ),
    [](const testing::TestParamInfo<TensorLiteralSadParam>& info) {
    return info.param.test_id;
    }
);
