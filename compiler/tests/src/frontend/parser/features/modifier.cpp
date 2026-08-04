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
            auto reg = [](const ConstructCase<ModifierVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "SingleModifier",
                .code = "@simple",
                .verifier = std::vector<ModifierSpec>{
                    {"simple"}
                }
            });

            reg({
                .name = "SingleModifierWithEmptyParens",
                .code = "@simple()",
                .verifier = std::vector<ModifierSpec>{
                    {"simple"}
                }
            });

            reg({
                .name = "MultipleModifiers",
                .code = "@first @second",
                .verifier = std::vector<ModifierSpec>{
                    {"first"},
                    {"second"}
                }
            });

            reg({
                .name = "ModifierWithOneArgument",
                .code = "@meta(version: 1)",
                .verifier = std::vector<ModifierSpec>{
                    {
                        "meta", {
                            {"version", IsNumber("1")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierWithMultipleArguments",
                .code = "@config(active: true, retries: 3, strategy: \"fast\")",
                .verifier = std::vector<ModifierSpec>{
                    {
                        "config", {
                            {"active", IsBoolean(true)},
                            {"retries", IsNumber("3")},
                            {"strategy", IsString("\"fast\"")}
                        }
                    }
                }
            });

            reg({
                .name = "MixedModifiers",
                .code = "@inline @deprecated(msg: \"old\") @export",
                .verifier = std::vector<ModifierSpec>{
                    {"inline"},
                    {
                        "deprecated", {
                            {"msg", IsString("\"old\"")}
                        }
                    },
                    {"export"}
                }
            });

            reg({
                .name = "ModifierWhitespaceAndCommentsCondensed",
                .code = "@test1 // line comment\n"
                "( \n a: 1 // end \n ) "
                "@\n test2(b: 2) "
                "@test3\n\n\n(c: 3) ",
                .verifier = std::vector<ModifierSpec>{
                    {"test1", {{"a", IsNumber("1")}}},
                    {"test2", {{"b", IsNumber("2")}}},
                    {"test3", {{"c", IsNumber("3")}}}
                }
            });

            return true;
        }();
    }

    TEST_P(ModifierRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidModifiers(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        Modifier,
        ModifierRegistryRunner,
        testing::ValuesIn(ConstructRegistry::modifiers()),
        TestNameGenerator{}
    );
}
