#include <gtest/gtest.h>

#include "../../../../include/errors/valuascript_exception.h"
#include "stages/parser/parser_stage.h"
#include "stages/parser/ast.h"
#include "stages/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserDictLiteralTestBase : public testing::Test {
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

struct DictLiteralHappyParam {
    std::string test_id;
    std::string source_code;
};

class DictLiteralHappyPathTest : public ParserDictLiteralTestBase,
                                 public testing::WithParamInterface<DictLiteralHappyParam> {
};

TEST_P(DictLiteralHappyPathTest, ParsesSuccessfully) {
    const DictLiteralHappyParam &param = GetParam();

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
    DictLiteralHappyPathTest,
    testing::Values(
        DictLiteralHappyParam{"dict", "{ name: \"one\", age: 20 }"},
        DictLiteralHappyParam{"dict_complex", "{ name: func_call(), age: 20 }"},
        DictLiteralHappyParam{"dict_complex_1", "{ name: func_call(), age: matrix[0][:] }"},
        DictLiteralHappyParam{"dict_complex_2", "{ name: func_call(), age: matrix[0][:], money: if a() then b else if c() then d(c: 12) else 10 }"}
    ),
    [](const testing::TestParamInfo<DictLiteralHappyParam>& info) {
    return info.param.test_id;
    }
);

struct DictLiteralSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class DictLiteralSadPathTest : public ParserDictLiteralTestBase,
                               public testing::WithParamInterface<DictLiteralSadParam> {
};

TEST_P(DictLiteralSadPathTest, ThrowsCorrectSyntaxError) {
    const DictLiteralSadParam &param = GetParam();

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
    DictLiteralSadPathTest,
    testing::Values(
        DictLiteralSadParam{"dict_missing_brace", "{a: 1", ErrorCode::
        UnmatchedBraceInDictionaryLiteral},
        DictLiteralSadParam{"dict_missing_key", "{1}", ErrorCode::
        ExpectedDictionaryKey},
        DictLiteralSadParam{"dict_missing_colon", "{a 1}", ErrorCode::
        ExpectedColonAfterDictionaryKey},
        DictLiteralSadParam{"dict_empty", "{a}", ErrorCode::
        ExpectedColonAfterDictionaryKey}
    ),
    [](const testing::TestParamInfo<DictLiteralSadParam>& info) {
    return info.param.test_id;
    }
);
