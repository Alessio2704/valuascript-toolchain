#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ExtensionDefinitionRegistryRunner : public ParserTestBase,
                                              public testing::WithParamInterface<RegistryEntry<ExtVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExtVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "MinimalExtension",
                .code = "extension TargetType {}",
                .verifier = IsExtensionDef({}, IsType("TargetType"), {})
            });

            reg({
                .name = "ModifiedExtension",
                .code = "@modifier extension TargetType {}",
                .verifier = IsExtensionDef({{"modifier"}}, IsType("TargetType"), {})
            });

            reg({
                .name = "ModifiedWithArgs",
                .code = "@modifier(p: 1) extension TargetType {}",
                .verifier = IsExtensionDef({{"modifier", {{"p", IsNumber("1")}}}}, IsType("TargetType"), {})
            });

            reg({
                .name = "ComplexTargetType",
                .code = "extension map<string, int> {}",
                .verifier = IsExtensionDef({}, IsType("map", IsType("string"), IsType("int")), {})
            });

            return true;
        }();
    }

    TEST_P(ExtensionDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidExtensionDefinition(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExtensionDefinition,
        ExtensionDefinitionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::extensions()),
        TestNameGenerator{}
    );
}
