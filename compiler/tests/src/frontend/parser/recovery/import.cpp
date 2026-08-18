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
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 13}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "MissingImportStringPath",
                .code = "import ",
                .errors = {PErr{.code = E::MissingImportPathString, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsImport("<error>"),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "ImportWithDocStringPath",
                .code = R"(import """file_path.vs""")",
                .errors = {PErr{.code = E::MissingImportPathString, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 26}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath1",
                .code = "import 1",
                .errors = {PErr{.code = E::MissingImportPathString, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath2",
                .code = "import identifier",
                .errors = {PErr{.code = E::MissingImportPathString, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 18}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath3",
                .code = "import if a then b else c",
                .errors = {PErr{.code = E::MissingImportPathString, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 10}},
                .verifier = IsImport("<error>")
            });

            reg({
                .name = "ImportInvalidStringPath4",
                .code = "import a()",
                .errors = {PErr{.code = E::MissingImportPathString, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}},
                .verifier = IsImport("<error>")
            });

            return true;
        }();
    }

    TEST_P(ImportErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& p = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + p.test_name);

        ExpectImportErrors(p.code, p.errors, p.verifier, p.skip_contexts, p.context_overrides, p.excluded_sentinels, p.accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Import,
        ImportErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::imports()),
        TestNameGenerator{}
    );
}
