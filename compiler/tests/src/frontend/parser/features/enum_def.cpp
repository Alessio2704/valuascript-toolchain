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
                .verifier = IsEnumDef("E", {}, IsType("int"), std::vector<EnumCaseSpec>{})
            });

            reg({
                .name = "MinimalEnum",
                .code = "enum E: int { A }",
                .verifier = IsEnumDef("E", {}, IsType("int"),
                    EnumCaseSpec{.name = "A"}
                )
            });

            reg({
                .name = "MultipleCases",
                .code = "enum Color: int { Red, Green, Blue }",
                .verifier = IsEnumDef("Color", {}, IsType("int"),
                    EnumCaseSpec{.name = "Red"},
                    EnumCaseSpec{.name = "Green"},
                    EnumCaseSpec{.name = "Blue"}
                )
            });

            reg({
                .name = "CasesWithExplicitValues",
                .code = "enum Status: int { Active = 1, Inactive = 0, Pending = 2 }",
                .verifier = IsEnumDef("Status", {}, IsType("int"),
                    EnumCaseSpec{.name = "Active", .value_v = IsNumber("1")},
                    EnumCaseSpec{.name = "Inactive", .value_v = IsNumber("0")},
                    EnumCaseSpec{.name = "Pending", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "MixedValuedAndUnvaluedCases",
                .code = "enum Flag: int { None = 0, First, Second, Last = 10 }",
                .verifier = IsEnumDef("Flag", {}, IsType("int"),
                    EnumCaseSpec{.name = "None", .value_v = IsNumber("0")},
                    EnumCaseSpec{.name = "First"},
                    EnumCaseSpec{.name = "Second"},
                    EnumCaseSpec{.name = "Last", .value_v = IsNumber("10")}
                )
            });

            reg({
                .name = "TrailingComma",
                .code = "enum E: int { A, B, }",
                .verifier = IsEnumDef("E", {}, IsType("int"),
                    EnumCaseSpec{.name = "A"},
                    EnumCaseSpec{.name = "B"}
                )
            });

            reg({
                .name = "InterleavingModifiedCases",
                .code = "enum E: int { @primary A, B, @deprecated C = 99 }",
                .verifier = IsEnumDef("E", {}, IsType("int"),
                    EnumCaseSpec{.name = "A", .modifiers = {{"primary"}}},
                    EnumCaseSpec{.name = "B"},
                    EnumCaseSpec{.name = "C", .modifiers = {{"deprecated"}}, .value_v = IsNumber("99")}
                )
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
                .verifier = IsEnumDef("State", {}, IsType("string"),
                    EnumCaseSpec{.name = "Open", .modifiers = {{"init"}}, .value_v = IsString("\"open\"")},
                    EnumCaseSpec{.name = "Closed", .value_v = IsString("\"closed\"")}
                )
            });

            return true;
        }();
    }

    TEST_P(EnumDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidEnumDefinition(entry.code, entry.verifier, entry.skip_contexts);
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
