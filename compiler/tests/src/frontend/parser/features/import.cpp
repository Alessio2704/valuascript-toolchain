#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ImportStatementRegistryRunner : public ParserTestBase,
                                          public testing::WithParamInterface<RegistryEntry<ImportVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ImportVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "ValidatesImportStatement1",
                .code = "import \"file.vs\"",
                .verifier = IsImport("\"file.vs\"")
            });

            reg({
                .name = "ValidatesImportStatement2",
                .code = "import \"path/to/file.vs\"",
                .verifier = IsImport("\"path/to/file.vs\"")
            });

            return true;
        }();
    }

    TEST_P(ImportStatementRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidImport(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        ImportStatement,
        ImportStatementRegistryRunner,
        testing::ValuesIn(ConstructRegistry::imports()),
        TestNameGenerator{}
    );
}
