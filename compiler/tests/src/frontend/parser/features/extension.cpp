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
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("MinimalExtension",
                "extension TargetType {}",
                IsExtensionDef({}, IsType("TargetType"), {}));

            reg("ModifiedExtension",
                "@modifier extension TargetType {}",
                IsExtensionDef({{"modifier"}}, IsType("TargetType"), {}));

            reg("ModifiedWithArgs",
                "@modifier(p: 1) extension TargetType {}",
                IsExtensionDef({{"modifier", {{"p", IsNumber("1")}}}}, IsType("TargetType"), {}));

            reg("ComplexTargetType",
                "extension map<string, int> {}",
                IsExtensionDef({}, IsType("map", {IsType("string"), IsType("int")}), {}));

            return true;
        }();
    }

    TEST_P(ExtensionDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidExtensionDefinition(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExtensionDefinition,
        ExtensionDefinitionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::extensions()),
        [](const testing::TestParamInfo<RegistryEntry<ExtVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
