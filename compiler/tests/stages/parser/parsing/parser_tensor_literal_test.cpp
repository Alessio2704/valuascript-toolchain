#include <gtest/gtest.h>

#include "../../../../include/errors/valuascript_exception.h"
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserTensorLiteralTestBase : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string &expression) {
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

struct TensorLiteralHappyParam {
    std::string test_id;
    std::string source_code;
};

class TensorLiteralHappyPathTest : public ParserTensorLiteralTestBase,
                            public testing::WithParamInterface<TensorLiteralHappyParam> {
};

TEST_P(TensorLiteralHappyPathTest, ParsesSuccessfully) {
    const TensorLiteralHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
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
    TensorLiteralHappyParam{"vector_literal_complex_2", "[(a, b, c()), d(a: 1), if e then f else g()]"}
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

class TensorLiteralSadPathTest : public ParserTensorLiteralTestBase,
                          public testing::WithParamInterface<TensorLiteralSadParam> {
};

TEST_P(TensorLiteralSadPathTest, ThrowsCorrectSyntaxError) {
    const TensorLiteralSadParam &param = GetParam();

    try {
        parse_code(param.source_code);
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
