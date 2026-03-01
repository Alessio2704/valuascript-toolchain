#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/parser/ast.h"
#include "stages/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserBracketAccessTestBase : public testing::Test {
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

struct BracketAccessHappyParam {
    std::string test_id;
    std::string source_code;
};

class BracketAccessHappyPathTest : public ParserBracketAccessTestBase,
                            public testing::WithParamInterface<BracketAccessHappyParam> {
};

TEST_P(BracketAccessHappyPathTest, ParsesSuccessfully) {
    const BracketAccessHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto &assignment = ast->execution_steps[0];
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
    ErrorCode expected_error;
};

class BracketAccessSadPathTest : public ParserBracketAccessTestBase,
                          public testing::WithParamInterface<BracketAccessSadParam> {
};

TEST_P(BracketAccessSadPathTest, ThrowsCorrectSyntaxError) {
    const BracketAccessSadParam &param = GetParam();

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
    BracketAccessSadPathTest,
    testing::Values(
        BracketAccessSadParam{"unclosed_vector_access", "vec[0", ErrorCode::UnmatchedBracket},
        BracketAccessSadParam{"empty_vector_access", "vec[]", ErrorCode::EmptyBracketAccess}
    ),
    [](const testing::TestParamInfo<BracketAccessSadParam>& info) {
    return info.param.test_id;
    }
);
