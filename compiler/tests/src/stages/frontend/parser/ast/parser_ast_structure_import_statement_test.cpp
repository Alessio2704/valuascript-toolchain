#include "../ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesImportStatement) {
        auto ast = parse_code("import \"path/to/file.vs\"");
        auto import = get_import(ast);

        ASSERT_NE(import, nullptr);
        EXPECT_EQ(import->path, "\"path/to/file.vs\"");
    }

    TEST_F(AstBaseTest, ValidatesMultipleImportStatements) {
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
}
