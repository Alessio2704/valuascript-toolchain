#include <gtest/gtest.h>

#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstImportStatementTest : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string &code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        auto parser_result = parser.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        });

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }

    ImportStatement *get_import(const std::shared_ptr<Program> &ast) {
        if (ast->import_statements.empty()) return nullptr;
        return ast->import_statements[0].get();
    }
};

TEST_F(AstImportStatementTest, ValidatesImportStatement) {
    auto ast = parse_code("import \"path/to/file.vs\"");
    auto import = get_import(ast);

    ASSERT_NE(import, nullptr);
    EXPECT_EQ(import->path, "\"path/to/file.vs\"");
}

TEST_F(AstImportStatementTest, ValidatesMultipleImportStatements) {
    std::string code =
            "import \"core/math.vs\"\n"
            "import \"models/dcf.vs\"\n"
            "let a = 1";

    auto ast = parse_code(code);

    EXPECT_EQ(ast->import_statements.size(), 2);
    EXPECT_EQ(ast->import_statements[0]->path, "\"core/math.vs\"");
    EXPECT_EQ(ast->import_statements[1]->path, "\"models/dcf.vs\"");
    EXPECT_EQ(ast->execution_steps.size(), 1);
}
