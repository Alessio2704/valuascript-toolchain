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
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("EmptyEnum",
                "enum E: int { }",
                IsEnumDef("E", {}, IsType("int"), {}));

            reg("MinimalEnum",
                "enum E: int { A }",
                IsEnumDef("E", {}, IsType("int"), {
                              {"A"}
                          }));

            reg("MultipleCases",
                "enum Color: int { Red, Green, Blue }",
                IsEnumDef("Color", {}, IsType("int"), {
                              {"Red"},
                              {"Green"},
                              {"Blue"}
                          }));

            reg("CasesWithExplicitValues",
                "enum Status: int { Active = 1, Inactive = 0, Pending = 2 }",
                IsEnumDef("Status", {}, IsType("int"), {
                              {"Active", {}, IsNumber("1")},
                              {"Inactive", {}, IsNumber("0")},
                              {"Pending", {}, IsNumber("2")}
                          }));

            reg("MixedValuedAndUnvaluedCases",
                "enum Flag: int { None = 0, First, Second, Last = 10 }",
                IsEnumDef("Flag", {}, IsType("int"), {
                              {"None", {}, IsNumber("0")},
                              {"First"},
                              {"Second"},
                              {"Last", {}, IsNumber("10")}
                          }));

            reg("TrailingComma",
                "enum E: int { A, B, }",
                IsEnumDef("E", {}, IsType("int"), {
                              {"A"},
                              {"B"}
                          }));

            reg("InterleavingModifiedCases",
                "enum E: int { @primary A, B, @deprecated C = 99 }",
                IsEnumDef("E", {}, IsType("int"), {
                              {"A", {{"primary"}}},
                              {"B"},
                              {"C", {{"deprecated"}}, IsNumber("99")}
                          }));

            reg("MultilineFormatting",
                "enum\n"
                "  State\n"
                "  : \n"
                "  string \n"
                "{\n"
                "  @init Open = \"open\",\n"
                "  Closed = \"closed\"\n"
                "}",
                IsEnumDef("State", {}, IsType("string"), {
                              {"Open", {{"init"}}, IsString("\"open\"")},
                              {"Closed", {}, IsString("\"closed\"")}
                          }));

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
        [](const testing::TestParamInfo<RegistryEntry<EnumVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
