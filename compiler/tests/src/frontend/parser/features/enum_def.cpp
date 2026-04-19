#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class EnumDefinitionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(EnumDefinitionSuccessPathTest, EmptyEnum)
    {
        ExpectValidParse(
            "enum E: int { }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("E", {}, IsType("int"), {})
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MinimalEnum)
    {
        ExpectValidParse(
            "enum E: int { A }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("E", {}, IsType("int"), {{"A"}})
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MultipleCases)
    {
        ExpectValidParse(
            "enum Color: int { Red, Green, Blue }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("Color", {}, IsType("int"), {
                                  {"Red"}, {"Green"}, {"Blue"}
                              })
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, CasesWithExplicitValues)
    {
        ExpectValidParse(
            "enum Status: int { Active = 1, Inactive = 0, Pending = 2 }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("Status", {}, IsType("int"), {
                                  {"Active", {}, IsNumber("1")},
                                  {"Inactive", {}, IsNumber("0")},
                                  {"Pending", {}, IsNumber("2")}
                              })
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MixedValuedAndUnvaluedCases)
    {
        ExpectValidParse(
            "enum Flag: int { None = 0, First, Second, Last = 10 }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("Flag", {}, IsType("int"), {
                                  {"None", {}, IsNumber("0")},
                                  {"First"},
                                  {"Second"},
                                  {"Last", {}, IsNumber("10")}
                              })
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, TrailingComma)
    {
        ExpectValidParse(
            "enum E: int { A, B, }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("E", {}, IsType("int"), {{"A"}, {"B"}})
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, InterleavingModifiedCases)
    {
        ExpectValidParse(
            "enum E: int { @primary A, B, @deprecated C = 99 }",
            ProgramSpec{
                .enums = {
                    IsEnumDef("E", {}, IsType("int"), {
                                  {"A", {{"primary"}}},
                                  {"B"},
                                  {"C", {{"deprecated"}}, IsNumber("99")}
                              })
                }
            }
        );
    }

    TEST_F(EnumDefinitionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidParse(
            "enum\n"
            "  State\n"
            "  : \n"
            "  string \n"
            "{\n"
            "  @init Open = \"open\",\n"
            "  Closed = \"closed\"\n"
            "}",
            ProgramSpec{
                .enums = {
                    IsEnumDef("State", {}, IsType("string"), {
                                  {"Open", {{"init"}}, IsString("\"open\"")},
                                  {"Closed", {}, IsString("\"closed\"")}
                              })
                }
            }
        );
    }
}
