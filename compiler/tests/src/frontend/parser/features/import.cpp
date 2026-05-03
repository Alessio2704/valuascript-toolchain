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
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("ValidatesImportStatement1",
                "import \"file.vs\"",
                IsImport("\"file.vs\""));

            reg("ValidatesImportStatement2",
                "import \"path/to/file.vs\"",
                IsImport("\"path/to/file.vs\""));

            return true;
        }();
    }

    TEST_P(ImportStatementRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidImport(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        ImportStatement,
        ImportStatementRegistryRunner,
        testing::ValuesIn(ConstructRegistry::imports()),
        [](const testing::TestParamInfo<RegistryEntry<ImportVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
