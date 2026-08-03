#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class TypeAliasRegistryRunner : public ParserTestBase,
                                    public testing::WithParamInterface<RegistryEntry<AliasVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<AliasVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "Simple",
                .code = "typealias Identifier = string",
                .verifier = IsTypeAlias("Identifier", {}, IsType("string"))
            });

            reg({
                .name = "MultilineFormatting",
                .code = "typealias\n"
                "Data\n "
                "= \n"
                "string\n",
                .verifier = IsTypeAlias("Data", {}, IsType("string"))
            });

            return true;
        }();
    }

    TEST_P(TypeAliasRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidTypeAlias(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAlias,
        TypeAliasRegistryRunner,
        testing::ValuesIn(ConstructRegistry::aliases()),
        [](const testing::TestParamInfo<RegistryEntry<AliasVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
