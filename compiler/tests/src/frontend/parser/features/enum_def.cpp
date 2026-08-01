#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class EnumDefinitionRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<RegistryEntry<EnumVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<EnumVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "EmptyEnum",
                .code = "enum E: int { }",
                .verifier = IsEnumDef("E", {}, IsType("int"), {})
            });

            reg({
                .name = "MinimalEnum",
                .code = "enum E: int { A }",
                .verifier = IsEnumDef("E", {}, IsType("int"), {
                    {"A"}
                })
            });

            reg({
                .name = "MultipleCases",
                .code = "enum Color: int { Red, Green, Blue }",
                .verifier = IsEnumDef("Color", {}, IsType("int"), {
                    {"Red"},
                    {"Green"},
                    {"Blue"}
                })
            });

            reg({
                .name = "CasesWithExplicitValues",
                .code = "enum Status: int { Active = 1, Inactive = 0, Pending = 2 }",
                .verifier = IsEnumDef("Status", {}, IsType("int"), {
                    {"Active", {}, IsNumber("1")},
                    {"Inactive", {}, IsNumber("0")},
                    {"Pending", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "MixedValuedAndUnvaluedCases",
                .code = "enum Flag: int { None = 0, First, Second, Last = 10 }",
                .verifier = IsEnumDef("Flag", {}, IsType("int"), {
                    {"None", {}, IsNumber("0")},
                    {"First"},
                    {"Second"},
                    {"Last", {}, IsNumber("10")}
                })
            });

            reg({
                .name = "TrailingComma",
                .code = "enum E: int { A, B, }",
                .verifier = IsEnumDef("E", {}, IsType("int"), {
                    {"A"},
                    {"B"}
                })
            });

            reg({
                .name = "InterleavingModifiedCases",
                .code = "enum E: int { @primary A, B, @deprecated C = 99 }",
                .verifier = IsEnumDef("E", {}, IsType("int"), {
                    {"A", {{"primary"}}},
                    {"B"},
                    {"C", {{"deprecated"}}, IsNumber("99")}
                })
            });

            reg({
                .name = "MultilineFormatting",
                .code = "enum\n"
                "  State\n"
                "  : \n"
                "  string \n"
                "{\n"
                "  @init Open = \"open\",\n"
                "  Closed = \"closed\"\n"
                "}",
                .verifier = IsEnumDef("State", {}, IsType("string"), {
                    {"Open", {{"init"}}, IsString("\"open\"")},
                    {"Closed", {}, IsString("\"closed\"")}
                })
            });

            return true;
        }();
    }

    TEST_P(EnumDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidEnumDefinition(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        EnumDefinition,
        EnumDefinitionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::enums()),
        [](const testing::TestParamInfo<RegistryEntry<EnumVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
