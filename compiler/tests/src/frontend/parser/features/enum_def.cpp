#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class EnumDefinitionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(EnumDefinitionSuccessPathTest, EmptyEnum)
    {
        ExpectValidEnumDefinition(
            "enum E: int { }",
            IsEnumDef("E", {}, IsType("int"), {})
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MinimalEnum)
    {
        ExpectValidEnumDefinition(
            "enum E: int { A }",
            IsEnumDef("E", {}, IsType("int"), {{"A"}})
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MultipleCases)
    {
        ExpectValidEnumDefinition(
            "enum Color: int { Red, Green, Blue }",
            IsEnumDef("Color", {}, IsType("int"), {
                          {"Red"}, {"Green"}, {"Blue"}
                      }
            )
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, CasesWithExplicitValues)
    {
        ExpectValidEnumDefinition(
            "enum Status: int { Active = 1, Inactive = 0, Pending = 2 }",
            IsEnumDef("Status", {}, IsType("int"), {
                          {"Active", {}, IsNumber("1")},
                          {"Inactive", {}, IsNumber("0")},
                          {"Pending", {}, IsNumber("2")}
                      }
            )
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MixedValuedAndUnvaluedCases)
    {
        ExpectValidEnumDefinition(
            "enum Flag: int { None = 0, First, Second, Last = 10 }",
            IsEnumDef("Flag", {}, IsType("int"), {
                          {"None", {}, IsNumber("0")},
                          {"First"},
                          {"Second"},
                          {"Last", {}, IsNumber("10")}
                      }
            )
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, TrailingComma)
    {
        ExpectValidEnumDefinition(
            "enum E: int { A, B, }",
            IsEnumDef("E", {}, IsType("int"), {{"A"}, {"B"}})
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, InterleavingModifiedCases)
    {
        ExpectValidEnumDefinition(
            "enum E: int { @primary A, B, @deprecated C = 99 }",
            IsEnumDef("E", {}, IsType("int"), {
                          {"A", {{"primary"}}},
                          {"B"},
                          {"C", {{"deprecated"}}, IsNumber("99")}
                      }
            )
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidEnumDefinition(
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
                      }
            )
        );
    }
}
