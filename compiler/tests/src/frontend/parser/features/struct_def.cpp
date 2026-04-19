#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class StructDefinitionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(StructDefinitionSuccessPathTest, MinimalStruct)
    {
        ExpectValidStructDefinition(
            "struct S {}",
            IsStructDef("S", {}, {})
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, SingleField)
    {
        ExpectValidStructDefinition(
            "struct S { f: int }",
            IsStructDef("S", {}, {
                            FieldSpec{"f", {}, IsType("int")}
                        }
            )
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, MultipleFields)
    {
        ExpectValidStructDefinition(
            "struct Point { x: float, y: float, z: float }",
            IsStructDef("Point", {}, {
                            FieldSpec{"x", {}, IsType("float")},
                            FieldSpec{"y", {}, IsType("float")},
                            FieldSpec{"z", {}, IsType("float")}
                        }
            )
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, TrailingComma)
    {
        ExpectValidStructDefinition(
            "struct S { f1: int, f2: bool, }",
            IsStructDef("S", {}, {
                            FieldSpec{"f1", {}, IsType("int")},
                            FieldSpec{"f2", {}, IsType("bool")}
                        }
            )
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, InterleavingModifiedFields)
    {
        ExpectValidStructDefinition(
            "struct User { @id id: int, username: string, @optional bio: string }",
            IsStructDef("User", {}, {
                            FieldSpec{"id", {{"id"}}, IsType("int")},
                            FieldSpec{"username", {}, IsType("string")},
                            FieldSpec{"bio", {{"optional"}}, IsType("string")}
                        }
            )
        );
    }

    TEST_F(StructDefinitionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidStructDefinition(
            "struct \n"
            "  Config \n"
            "{\n"
            "  @internal \n"
            "  secret: string, \n"
            "  \n"
            "  version: int \n"
            "}",
            IsStructDef("Config", {}, {
                            FieldSpec{"secret", {{"internal"}}, IsType("string")},
                            FieldSpec{"version", {}, IsType("int")}
                        }
            )
        );
    }
}
