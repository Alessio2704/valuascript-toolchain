#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserImportStatementTestBase : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string &expression) {
        std::string code = expression;

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

struct ImportStatementHappyParam {
    std::string test_id;
    std::string source_code;
    std::string path;
};

class ImportStatementHappyPathTest : public ParserImportStatementTestBase,
                           public testing::WithParamInterface<ImportStatementHappyParam> {
};

TEST_P(ImportStatementHappyPathTest, ParsesSuccessfully) {
    const ImportStatementHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 0);
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);
        EXPECT_EQ(ast->import_statements.size(), 1)  << "Expected exactly 1 import statement in AST.";
        EXPECT_EQ(ast->import_statements[0]->path, param.path);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    ImportStatementHappyPathTest,
    testing::Values(
        ImportStatementHappyParam{"1", "import \"file/path\"", "\"file/path\""},
        ImportStatementHappyParam{"2", "import \"file/path/module.vs\"", "\"file/path/module.vs\""}
    ),
    [](const testing::TestParamInfo<ImportStatementHappyParam>& info) {
    return info.param.test_id;
    }
);

struct ImportStatementSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class ImportStatementSadPathTest : public ParserImportStatementTestBase,
                         public testing::WithParamInterface<ImportStatementSadParam> {
};

TEST_P(ImportStatementSadPathTest, ThrowsCorrectSyntaxError) {
    const ImportStatementSadParam &param = GetParam();

    try {
        parse_code(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ErrorCategory::Syntax);
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    ImportStatementSadPathTest,
    testing::Values(
    ImportStatementSadParam{"missing_import", " \"file/path\"", ErrorCode::UnexpectedToken},
    ImportStatementSadParam{"docstring", "import \"\"\"file/path\"\"\"", ErrorCode::MissingImportPathString},
    ImportStatementSadParam{"identifier", "import identifier", ErrorCode::MissingImportPathString},
    ImportStatementSadParam{"number", "import 1", ErrorCode::MissingImportPathString},
    ImportStatementSadParam{"expression", "import if a then b else c", ErrorCode::MissingImportPathString},
    ImportStatementSadParam{"func_call", "import a()", ErrorCode::MissingImportPathString}
    ),
    [](const testing::TestParamInfo<ImportStatementSadParam>& info) {
    return info.param.test_id;
    }
);
