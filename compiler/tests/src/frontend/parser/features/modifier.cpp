#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ModifierRegistryRunner : public ParserTestBase,
                                   public testing::WithParamInterface<RegistryEntry<ModifierVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("SingleModifier",
                "@simple",
                std::vector<ModifierSpec>{
                    {"simple"}
                });

            reg("SingleModifierWithEmptyParens",
                "@simple()",
                std::vector<ModifierSpec>{
                    {"simple"}
                });

            reg("MultipleModifiers",
                "@first @second",
                std::vector<ModifierSpec>{
                    {"first"},
                    {"second"}
                });

            reg("ModifierWithOneArgument",
                "@meta(version: 1)",
                std::vector<ModifierSpec>{
                    {
                        "meta", {
                            {"version", IsNumber("1")}
                        }
                    }
                });

            reg("ModifierWithMultipleArguments",
                "@config(active: true, retries: 3, strategy: \"fast\")",
                std::vector<ModifierSpec>{
                    {
                        "config", {
                            {"active", IsBoolean(true)},
                            {"retries", IsNumber("3")},
                            {"strategy", IsString("\"fast\"")}
                        }
                    }
                });

            reg("MixedModifiers",
                "@inline @deprecated(msg: \"old\") @export",
                std::vector<ModifierSpec>{
                    {"inline"},
                    {
                        "deprecated", {
                            {"msg", IsString("\"old\"")}
                        }
                    },
                    {"export"}
                });

            reg("MultilineModifiers",
                "@export\n@meta(version: 2)\n",
                std::vector<ModifierSpec>{
                    {"export"},
                    {
                        "meta", {
                            {"version", IsNumber("2")}
                        }
                    }
                });

            return true;
        }();
    }

    TEST_P(ModifierRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidModifiers(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Modifier,
        ModifierRegistryRunner,
        testing::ValuesIn(ConstructRegistry::modifiers()),
        [](const testing::TestParamInfo<RegistryEntry<ModifierVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
