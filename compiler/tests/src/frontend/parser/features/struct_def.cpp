#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class StructDefinitionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(StructDefinitionSuccessPathTest, MinimalStruct)
    {
        ExpectValidParse(
            "struct S {}",
            ProgramSpec{
                .structs = {
                    IsStructDef("S", {}, {})
                }
            }
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, SingleField)
    {
        ExpectValidParse(
            "struct S { f: int }",
            ProgramSpec{
                .structs = {
                    IsStructDef("S", {}, {
                                    FieldSpec{"f", {}, IsType("int")}
                                })
                }
            }
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, MultipleFields)
    {
        ExpectValidParse(
            "struct Point { x: float, y: float, z: float }",
            ProgramSpec{
                .structs = {
                    IsStructDef("Point", {}, {
                                    FieldSpec{"x", {}, IsType("float")},
                                    FieldSpec{"y", {}, IsType("float")},
                                    FieldSpec{"z", {}, IsType("float")}
                                })
                }
            }
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, TrailingComma)
    {
        ExpectValidParse(
            "struct S { f1: int, f2: bool, }",
            ProgramSpec{
                .structs = {
                    IsStructDef("S", {}, {
                                    FieldSpec{"f1", {}, IsType("int")},
                                    FieldSpec{"f2", {}, IsType("bool")}
                                })
                }
            }
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, InterleavingModifiedFields)
    {
        ExpectValidParse(
            "struct User { @id id: int, username: string, @optional bio: string }",
            ProgramSpec{
                .structs = {
                    IsStructDef("User", {}, {
                                    FieldSpec{"id", {{"id"}}, IsType("int")},
                                    FieldSpec{"username", {}, IsType("string")},
                                    FieldSpec{"bio", {{"optional"}}, IsType("string")}
                                })
                }
            }
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidParse(
            "struct \n"
            "  Config \n"
            "{\n"
            "  @internal \n"
            "  secret: string, \n"
            "  \n"
            "  version: int \n"
            "}",
            ProgramSpec{
                .structs = {
                    IsStructDef("Config", {}, {
                                    FieldSpec{"secret", {{"internal"}}, IsType("string")},
                                    FieldSpec{"version", {}, IsType("int")}
                                })
                }
            }
        );
    }
}
