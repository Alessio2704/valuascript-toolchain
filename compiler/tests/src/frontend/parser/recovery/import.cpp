#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ImportErrorRegistryRunner : public ParserTestBase,
                                      public testing::WithParamInterface<ErrorRegistryEntry<ImportVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ImportVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingImportKeyword", "file_path.vs",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 13}},
                IsNull());

            reg("MissingImportStringPath", "import ",
                {{E::MissingImportPathString, 1, 7, 1, 8}},
                IsImport("<error>"));

            reg("ImportWithDocStringPath", R"(import """file_path.vs""")",
                {{E::MissingImportPathString, 1, 8, 1, 26}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath1", "import 1",
                {{E::MissingImportPathString, 1, 8, 1, 9}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath2", "import identifier",
                {{E::MissingImportPathString, 1, 8, 1, 18}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath3", "import if a then b else c",
                {{E::MissingImportPathString, 1, 8, 1, 10}},
                IsImport("<error>"));

            reg("ImportInvalidStringPath4", "import a()",
                {{E::MissingImportPathString, 1, 8, 1, 9}},
                IsImport("<error>"));

            return true;
        }();
    }

    TEST_P(ImportErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectImportErrors(code, errors, verifier, skip_contexts);
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
