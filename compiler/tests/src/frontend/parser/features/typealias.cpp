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
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("Simple",
                "typealias Identifier = string",
                IsTypeAlias("Identifier", {},
                            IsType("string")
                ));

            reg("MultilineFormatting",
                "typealias\n"
                "Data\n "
                "= \n"
                "string\n",
                IsTypeAlias("Data", {}, IsType("string")));

            return true;
        }();
    }

    TEST_P(TypeAliasRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidTypeAlias(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAlias,
        TypeAliasRegistryRunner,
        testing::ValuesIn(ConstructRegistry::aliases()),
        [](const testing::TestParamInfo<RegistryEntry<AliasVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
