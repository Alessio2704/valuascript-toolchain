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
                    EnumCaseSpec{"A"}
                )
            });

            reg({
                .name = "MultipleCases",
                .code = "enum Color: int { Red, Green, Blue }",
                .verifier = IsEnumDef("Color", {}, IsType("int"),
                    EnumCaseSpec{"Red"},
                    EnumCaseSpec{"Green"},
                    EnumCaseSpec{"Blue"}
                )
            });

            reg({
                .name = "CasesWithExplicitValues",
                .code = "enum Status: int { Active = 1, Inactive = 0, Pending = 2 }",
                .verifier = IsEnumDef("Status", {}, IsType("int"),
                    EnumCaseSpec{"Active", IsNumber("1")},
                    EnumCaseSpec{"Inactive", IsNumber("0")},
                    EnumCaseSpec{"Pending", IsNumber("2")}
                )
            });

            reg({
                .name = "MixedValuedAndUnvaluedCases",
                .code = "enum Flag: int { None = 0, First, Second, Last = 10 }",
                .verifier = IsEnumDef("Flag", {}, IsType("int"),
                    EnumCaseSpec{"None", IsNumber("0")},
                    EnumCaseSpec{"First"},
                    EnumCaseSpec{"Second"},
                    EnumCaseSpec{"Last", IsNumber("10")}
                )
            });

            reg({
                .name = "TrailingComma",
                .code = "enum E: int { A, B, }",
                .verifier = IsEnumDef("E", {}, IsType("int"),
                    EnumCaseSpec{"A"},
                    EnumCaseSpec{"B"}
                )
            });

            reg({
                .name = "InterleavingModifiedCases",
                .code = "enum E: int { @primary A, B, @deprecated C = 99 }",
                .verifier = IsEnumDef("E", {}, IsType("int"),
                    EnumCaseSpec{"A", std::vector<ModifierSpec>{{"primary"}}},
                    EnumCaseSpec{"B"},
                    EnumCaseSpec{"C", std::vector<ModifierSpec>{{"deprecated"}}, IsNumber("99")}
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
                    EnumCaseSpec{"Open", std::vector<ModifierSpec>{{"init"}}, IsString("\"open\"")},
                    EnumCaseSpec{"Closed", IsString("\"closed\"")}
                )
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
