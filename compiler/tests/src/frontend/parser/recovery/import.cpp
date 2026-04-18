#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ImportErrorTest : public ParserTestBase
    {
    };

    TEST_F(ImportErrorTest, MissingImportKeyword)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 11, 1, 13);

        ExpectParseErrorsWithRecovery(
            "file_path.vs",
            errors,
            ProgramSpec{
                .imports = {}
            }
        );
    }

    TEST_F(ImportErrorTest, MissingImportStringPath)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingImportPathString, 1, 7, 1, 8);

        ExpectParseErrorsWithRecovery(
            "import ",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("<error>")
                }
            }
        );
    }

    TEST_F(ImportErrorTest, ImportWithDocStringPath)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 26);

        ExpectParseErrorsWithRecovery(
            R"(import """file_path.vs""")",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("<error>")
                }
            }
        );
    }

    TEST_F(ImportErrorTest, ImportInvalidStringPath1)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 9);

        ExpectParseErrorsWithRecovery(
            "import 1",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("<error>")
                }
            }
        );
    }

    TEST_F(ImportErrorTest, ImportInvalidStringPath2)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 18);

        ExpectParseErrorsWithRecovery(
            "import identifier",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("<error>")
                }
            }
        );
    }

    TEST_F(ImportErrorTest, ImportInvalidStringPath3)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 10);

        ExpectParseErrorsWithRecovery(
            "import if a then b else c",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("<error>")
                }
            }
        );
    }

    TEST_F(ImportErrorTest, ImportInvalidStringPath4)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 9);

        ExpectParseErrorsWithRecovery(
            "import a()",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("<error>")
                }
            }
        );
    }
}
