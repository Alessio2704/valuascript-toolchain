#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ImportErrorRegistryRunner : public ParserTestBase,
                                      public testing::WithParamInterface<ErrorRegistryEntry<ImportVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ImportVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingImportKeyword", "file_path.vs",
                {{ValuascriptErrorCode::InvalidStandaloneStatement, 1, 11, 1, 13}},
                IsNull());

            reg("MissingImportStringPath", "import ",
                {{ValuascriptErrorCode::MissingImportPathString, 1, 7, 1, 8}},
                IsImport("<error>"));

            reg("ImportWithDocStringPath", R"(import """file_path.vs""")",
                {{ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 26}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath1", "import 1",
                {{ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 9}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath2", "import identifier",
                {{ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 18}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath3", "import if a then b else c",
                {{ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 10}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath4", "import a()",
                {{ValuascriptErrorCode::MissingImportPathString, 1, 8, 1, 9}},
                IsImport("<error>"));

            return true;
        }();
    }

    TEST_P(ImportErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectImportErrors(code, errors, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Import,
        ImportErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::imports()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<ImportVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
