#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ImportStatementSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ImportStatementSuccessPathTest, ValidatesImportStatement1)
    {
        ExpectValidParse(
            "import \"file.vs\"",
            ProgramSpec{
                .imports = {
                    IsImport("\"file.vs\"")
                }
            }
        );
    }

    TEST_F(ImportStatementSuccessPathTest, ValidatesImportStatement2)
    {
        ExpectValidParse(
            "import \"path/to/file.vs\"",
            ProgramSpec{
                .imports = {
                    IsImport("\"path/to/file.vs\"")
                }
            }
        );
    }

    TEST_F(ImportStatementSuccessPathTest, ValidatesMultipleImportStatements)
    {
        ExpectValidParse(
            "import \"core/math.vs\"\n"
            "import \"models/dcf.vs\"\n"
            "let a = 1",
            ProgramSpec{
                .imports = {
                    IsImport("\"core/math.vs\""),
                    IsImport("\"models/dcf.vs\"")
                },
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }
}
