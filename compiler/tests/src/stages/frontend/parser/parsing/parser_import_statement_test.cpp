#include <gtest/gtest.h>
#include "frontend/parser/ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct ImportStatementHappyParam {
        std::string test_name;
        std::string source_code;
        std::string path;
    };

    class ImportStatementHappyPathTest : public AstBaseTest,
                                         public testing::WithParamInterface<ImportStatementHappyParam> {
    };

    TEST_P(ImportStatementHappyPathTest, ParsesSuccessfully) {
        const ImportStatementHappyParam &param = GetParam();

        std::shared_ptr<Program> ast;
        EXPECT_NO_THROW({
            ast = parse_code(param.source_code);
            }) << "Parser threw an exception on valid assignment test: " << param.test_name;

        if (ast) {
            ASSERT_EQ(ast->execution_steps.size(), 0);
            EXPECT_EQ(ast->directives.size(), 0);
            EXPECT_EQ(ast->function_definitions.size(), 0);
            EXPECT_EQ(ast->import_statements.size(), 1) << "Expected exactly 1 import statement in AST.";
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
        return info.param.test_name;
        }
    );

    struct ImportStatementSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class ImportStatementSadPathTest : public AstBaseTest,
                                       public testing::WithParamInterface<ImportStatementSadParam> {
    };

    TEST_P(ImportStatementSadPathTest, ThrowsCorrectSyntaxError) {
        const ImportStatementSadParam &param = GetParam();

        try {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax);
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        ImportStatementSadPathTest,
        testing::Values(
            ImportStatementSadParam{"missing_import", " \"file/path\"", ValuascriptErrorCode::InvalidStandaloneStatement},
            ImportStatementSadParam{"docstring", "import \"\"\"file/path\"\"\"", ValuascriptErrorCode::
            MissingImportPathString},
            ImportStatementSadParam{"identifier", "import identifier", ValuascriptErrorCode::MissingImportPathString},
            ImportStatementSadParam{"number", "import 1", ValuascriptErrorCode::MissingImportPathString},
            ImportStatementSadParam{"expression", "import if a then b else c", ValuascriptErrorCode::
            MissingImportPathString},
            ImportStatementSadParam{"func_call", "import a()", ValuascriptErrorCode::MissingImportPathString}
        ),
        [](const testing::TestParamInfo<ImportStatementSadParam>& info) {
        return info.param.test_name;
        }
    );
}
