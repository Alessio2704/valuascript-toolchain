#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserTupleTestBase : public testing::Test {
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

struct TupleHappyParam {
    std::string test_id;
    std::string source_code;
};

class TupleHappyPathTest : public ParserTupleTestBase,
                           public testing::WithParamInterface<TupleHappyParam> {
};

TEST_P(TupleHappyPathTest, ParsesSuccessfully) {
    const TupleHappyParam &param = GetParam();

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

class TupleSadPathTest : public ParserTupleTestBase,
                         public testing::WithParamInterface<TupleSadParam> {
};

TEST_P(TupleSadPathTest, ThrowsCorrectSyntaxError) {
    const TupleSadParam &param = GetParam();

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
    TupleSadPathTest,
    testing::Values(
    TupleSadParam{"tuple_missing_second_value", "(a, ", ErrorCode::InvalidExpression},
    TupleSadParam{"tuple_parenthesis", "(a, b", ErrorCode::UnmatchedParenthesisInTuple}
    ),
    [](const testing::TestParamInfo<TupleSadParam>& info) {
    return info.param.test_id;
    }
);
