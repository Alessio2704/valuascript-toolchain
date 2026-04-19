#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ImportStatementSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ImportStatementSuccessPathTest, ValidatesImportStatement1)
    {
        ExpectValidImport(
            "import \"file.vs\"",
            IsImport("\"file.vs\"")
        );
    }

    TEST_F(ImportStatementSuccessPathTest, ValidatesImportStatement2)
    {
        ExpectValidImport(
            "import \"path/to/file.vs\"",
            IsImport("\"path/to/file.vs\"")
        );
    }
}
