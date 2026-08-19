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
                    {.name="simple"}
                }
            });

            reg({
                .name = "SingleModifierWithEmptyParens",
                .code = "@simple()",
                .verifier = std::vector<ModifierSpec>{
                    {.name="simple"}
                }
            });

            reg({
                .name = "MultipleModifiers",
                .code = "@first @second",
                .verifier = std::vector<ModifierSpec>{
                    {.name="first"},
                    {.name="second"}
                }
            });

            reg({
                .name = "ModifierWithOneArgument",
                .code = "@meta(version: 1)",
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="meta", .args={
                            {.label="version", .value_v=IsNumber("1")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierWithMultipleArguments",
                .code = "@config(active: true, retries: 3, strategy: \"fast\")",
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="config", .args={
                            {.label="active", .value_v=IsBoolean(true)},
                            {.label="retries", .value_v=IsNumber("3")},
                            {.label="strategy", .value_v=IsString("\"fast\"")}
                        }
                    }
                }
            });

            reg({
                .name = "MixedModifiers",
                .code = "@inline @deprecated(msg: \"old\") @export",
                .verifier = std::vector<ModifierSpec>{
                    {.name="inline"},
                    {
                        .name="deprecated", .args={
                            {.label="msg", .value_v=IsString("\"old\"")}
                        }
                    },
                    {.name="export"}
                }
            });

            reg({
                .name = "ModifierWhitespaceAndCommentsCondensed",
                .code = "@test1 // line comment\n"
                "( \n a: 1 // end \n ) "
                "@\n test2(b: 2) "
                "@test3\n\n\n(c: 3) ",
                .verifier = std::vector<ModifierSpec>{
                    {.name="test1", .args={{.label="a", .value_v=IsNumber("1")}}},
                    {.name="test2", .args={{.label="b", .value_v=IsNumber("2")}}},
                    {.name="test3", .args={{.label="c", .value_v=IsNumber("3")}}}
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
