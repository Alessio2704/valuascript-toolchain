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
            auto reg = [](const RecoveryCase<ImportVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingImportKeyword",
                .code = "file_path.vs",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 13}},
                .verifier = IsNull()
            });

            reg({
                .name = "MissingImportStringPath",
                .code = "import ",
                .errors = {{E::MissingImportPathString, 1, 7, 1, 8}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportWithDocStringPath",
                .code = R"(import """file_path.vs""")",
                .errors = {{E::MissingImportPathString, 1, 8, 1, 26}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath1",
                .code = "import 1",
                .errors = {{E::MissingImportPathString, 1, 8, 1, 9}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath2",
                .code = "import identifier",
                .errors = {{E::MissingImportPathString, 1, 8, 1, 18}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath3",
                .code = "import if a then b else c",
                .errors = {{E::MissingImportPathString, 1, 8, 1, 10}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath4",
                .code = "import a()",
                .errors = {{E::MissingImportPathString, 1, 8, 1, 9}},
                .verifier = IsImport("<error>")
            });

            return true;
        }();
    }

    TEST_P(ImportErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectImportErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
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
