#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
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
