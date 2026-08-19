#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstDeclSpanTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstDeclSpanTest, FunctionDefinitionWithoutModifiers)
    {
        std::string code =
                "func add(a: int, b: int) -> int {\n"
                "    return a + b\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("add",
                    /*modifiers=*/{},
                    /*params=*/{
                        ParamSpec{
                            .name = "a",
                            .type_v = IsType("int").with_name_span(1, 13, 1, 16).with_span(1, 13, 1, 16)
                        }.with_name_span(1, 10, 1, 11)
                         .with_span(1, 10, 1, 16),
                        ParamSpec{
                            .name = "b",
                            .type_v = IsType("int").with_name_span(1, 21, 1, 24).with_span(1, 21, 1, 24)
                        }.with_name_span(1, 18, 1, 19)
                         .with_span(1, 18, 1, 24)
                    },
                    /*returns=*/{
                        IsType("int").with_name_span(1, 29, 1, 32).with_span(1, 29, 1, 32)
                    },
                    /*body=*/{
                        IsReturn(
                            IsBinary(TokenType::Plus,
                                IsIdentifier("a").with_span(2, 12, 2, 13),
                                IsIdentifier("b").with_span(2, 16, 2, 17)
                            ).with_span(2, 12, 2, 17)
                        ).with_span(2, 5, 2, 17)
                    }
                ).with_name_span(1, 6, 1, 9)
                 .with_span(1, 1, 3, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, FunctionDefinitionWithSingleModifier)
    {
        std::string code =
                "@inline\n"
                "func fastCalc() -> int {\n"
                "    return 42\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("fastCalc",
                    /*modifiers=*/{
                        ModifierSpec{.name = "inline"}
                            .with_name_span(1, 2, 1, 8)
                            .with_span(1, 1, 1, 8)
                    },
                    /*params=*/{},
                    /*returns=*/{
                        IsType("int").with_name_span(2, 20, 2, 23).with_span(2, 20, 2, 23)
                    },
                    /*body=*/{
                        IsReturn(IsNumber("42").with_span(3, 12, 3, 14))
                            .with_span(3, 5, 3, 14)
                    }
                ).with_name_span(2, 6, 2, 14)
                 .with_span(1, 1, 4, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, FunctionDefinitionWithMultipleModifiersAndArguments)
    {
        std::string code =
                "@route(path: \"/users\", cache: true)\n"
                "@timeout(ms: 5000)\n"
                "func getUsers() -> int {\n"
                "    return 1\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("getUsers",
                    /*modifiers=*/{
                        ModifierSpec{
                            .name = "route",
                            .args = {
                                ArgSpec{.label = "path", .value_v = IsString("\"/users\"").with_span(1, 14, 1, 22)}
                                    .with_name_span(1, 8, 1, 12)
                                    .with_span(1, 8, 1, 22),
                                ArgSpec{.label = "cache", .value_v = IsBoolean(true).with_span(1, 31, 1, 35)}
                                    .with_name_span(1, 24, 1, 29)
                                    .with_span(1, 24, 1, 35)
                            }
                        }.with_name_span(1, 2, 1, 7)
                         .with_span(1, 1, 1, 36),
                        ModifierSpec{
                            .name = "timeout",
                            .args = {
                                ArgSpec{.label = "ms", .value_v = IsNumber("5000").with_span(2, 14, 2, 18)}
                                    .with_name_span(2, 10, 2, 12)
                                    .with_span(2, 10, 2, 18)
                            }
                        }.with_name_span(2, 2, 2, 9)
                         .with_span(2, 1, 2, 19)
                    },
                    /*params=*/{},
                    /*returns=*/{
                        IsType("int").with_name_span(3, 20, 3, 23).with_span(3, 20, 3, 23)
                    },
                    /*body=*/{
                        IsReturn(IsNumber("1").with_span(4, 12, 4, 13))
                            .with_span(4, 5, 4, 13)
                    }
                ).with_name_span(3, 6, 3, 14)
                 .with_span(1, 1, 5, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, FunctionDefinitionWithParamModifiersAndDefaultValues)
    {
        std::string code =
                "func calculateSum(firstVal: int, @attr secondVal: int = 10) -> int {\n"
                "    return firstVal + secondVal\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("calculateSum",
                    /*modifiers=*/{},
                    /*params=*/{
                        ParamSpec{
                            .name = "firstVal",
                            .type_v = IsType("int").with_name_span(1, 29, 1, 32).with_span(1, 29, 1, 32)
                        }.with_name_span(1, 19, 1, 27)
                         .with_span(1, 19, 1, 32),
                        ParamSpec{
                            .name = "secondVal",
                            .modifiers = {
                                ModifierSpec{.name = "attr"}.with_name_span(1, 35, 1, 39).with_span(1, 34, 1, 39)
                            },
                            .type_v = IsType("int").with_name_span(1, 51, 1, 54).with_span(1, 51, 1, 54),
                            .default_v = IsNumber("10").with_span(1, 57, 1, 59)
                        }.with_name_span(1, 40, 1, 49)
                         .with_span(1, 34, 1, 59)
                    },
                    /*returns=*/{
                        IsType("int").with_name_span(1, 64, 1, 67).with_span(1, 64, 1, 67)
                    },
                    /*body=*/{
                        IsReturn(
                            IsBinary(TokenType::Plus,
                                IsIdentifier("firstVal").with_span(2, 12, 2, 20),
                                IsIdentifier("secondVal").with_span(2, 23, 2, 32)
                            ).with_span(2, 12, 2, 32)
                        ).with_span(2, 5, 2, 32)
                    }
                ).with_name_span(1, 6, 1, 18)
                 .with_span(1, 1, 3, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, FunctionDefinitionWithMultipleReturnTypes)
    {
        std::string code =
                "func split(val: int) -> int, string {\n"
                "    return val, \"ok\"\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("split",
                    /*modifiers=*/{},
                    /*params=*/{
                        ParamSpec{
                            .name = "val",
                            .type_v = IsType("int").with_name_span(1, 17, 1, 20).with_span(1, 17, 1, 20)
                        }.with_name_span(1, 12, 1, 15)
                         .with_span(1, 12, 1, 20)
                    },
                    /*returns=*/{
                        IsType("int").with_name_span(1, 25, 1, 28).with_span(1, 25, 1, 28),
                        IsType("string").with_name_span(1, 30, 1, 36).with_span(1, 30, 1, 36)
                    },
                    /*body=*/{
                        IsReturn(
                            IsIdentifier("val").with_span(2, 12, 2, 15),
                            IsString("\"ok\"").with_span(2, 17, 2, 21)
                        ).with_span(2, 5, 2, 21)
                    }
                ).with_name_span(1, 6, 1, 11)
                 .with_span(1, 1, 3, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, StructDefinitionWithoutModifiers)
    {
        std::string code =
                "struct Point {\n"
                "    x: float,\n"
                "    y: float\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .structs = {
                IsStructDef("Point",
                    FieldSpec{
                        .name = "x",
                        .type_v = IsType("float").with_name_span(2, 8, 2, 13).with_span(2, 8, 2, 13)
                    }.with_name_span(2, 5, 2, 6)
                     .with_span(2, 5, 2, 13),
                    FieldSpec{
                        .name = "y",
                        .type_v = IsType("float").with_name_span(3, 8, 3, 13).with_span(3, 8, 3, 13)
                    }.with_name_span(3, 5, 3, 6)
                     .with_span(3, 5, 3, 13)
                ).with_name_span(1, 8, 1, 13)
                 .with_span(1, 1, 4, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, StructDefinitionWithSingleModifier)
    {
        std::string code =
                "@serializable\n"
                "struct User {\n"
                "    id: int\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .structs = {
                IsStructDef("User",
                    /*modifiers=*/{
                        ModifierSpec{.name = "serializable"}
                            .with_name_span(1, 2, 1, 14)
                            .with_span(1, 1, 1, 14)
                    },
                    FieldSpec{
                        .name = "id",
                        .type_v = IsType("int").with_name_span(3, 9, 3, 12).with_span(3, 9, 3, 12)
                    }.with_name_span(3, 5, 3, 7)
                     .with_span(3, 5, 3, 12)
                ).with_name_span(2, 8, 2, 12)
                 .with_span(1, 1, 4, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, StructDefinitionWithMultipleModifiersAndFields)
    {
        std::string code =
                "@table(name: \"users\")\n"
                "@cacheable\n"
                "struct UserRecord {\n"
                "    id: int,\n"
                "    @primary @indexed\n"
                "    email: string\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .structs = {
                IsStructDef("UserRecord",
                    /*modifiers=*/{
                        ModifierSpec{
                            .name = "table",
                            .args = {
                                ArgSpec{.label = "name", .value_v = IsString("\"users\"").with_span(1, 14, 1, 21)}
                                    .with_name_span(1, 8, 1, 12)
                                    .with_span(1, 8, 1, 21)
                            }
                        }.with_name_span(1, 2, 1, 7)
                         .with_span(1, 1, 1, 22),
                        ModifierSpec{.name = "cacheable"}
                            .with_name_span(2, 2, 2, 11)
                            .with_span(2, 1, 2, 11)
                    },
                    FieldSpec{
                        .name = "id",
                        .type_v = IsType("int").with_name_span(4, 9, 4, 12).with_span(4, 9, 4, 12)
                    }.with_name_span(4, 5, 4, 7)
                     .with_span(4, 5, 4, 12),
                    FieldSpec{
                        .name = "email",
                        .modifiers = {
                            ModifierSpec{.name = "primary"}.with_name_span(5, 6, 5, 13).with_span(5, 5, 5, 13),
                            ModifierSpec{.name = "indexed"}.with_name_span(5, 15, 5, 22).with_span(5, 14, 5, 22)
                        },
                        .type_v = IsType("string").with_name_span(6, 12, 6, 18).with_span(6, 12, 6, 18)
                    }.with_name_span(6, 5, 6, 10)
                     .with_span(5, 5, 6, 18)
                ).with_name_span(3, 8, 3, 18)
                 .with_span(1, 1, 7, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, EnumDefinitionWithoutModifiers)
    {
        std::string code =
                "enum Status: int {\n"
                "    Active = 1,\n"
                "    Inactive = 0\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .enums = {
                IsEnumDef("Status",
                    /*modifiers=*/{},
                    /*type=*/IsType("int").with_name_span(1, 14, 1, 17).with_span(1, 14, 1, 17),
                    /*cases=*/
                    EnumCaseSpec{
                        .name = "Active",
                        .value_v = IsNumber("1").with_span(2, 14, 2, 15)
                    }.with_name_span(2, 5, 2, 11)
                     .with_span(2, 5, 2, 15),
                    EnumCaseSpec{
                        .name = "Inactive",
                        .value_v = IsNumber("0").with_span(3, 16, 3, 17)
                    }.with_name_span(3, 5, 3, 13)
                     .with_span(3, 5, 3, 17)
                ).with_name_span(1, 6, 1, 12)
                 .with_span(1, 1, 4, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, EnumDefinitionWithModifiersAndExplicitValues)
    {
        std::string code =
                "@flag_enum\n"
                "enum Permission: int {\n"
                "    @initial Read = 1,\n"
                "    Write = 2\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .enums = {
                IsEnumDef("Permission",
                    /*modifiers=*/{
                        ModifierSpec{.name = "flag_enum"}
                            .with_name_span(1, 2, 1, 11)
                            .with_span(1, 1, 1, 11)
                    },
                    /*type=*/IsType("int").with_name_span(2, 18, 2, 21).with_span(2, 18, 2, 21),
                    /*cases=*/
                    EnumCaseSpec{
                        .name = "Read",
                        .modifiers = {
                            ModifierSpec{.name = "initial"}.with_name_span(3, 6, 3, 13).with_span(3, 5, 3, 13)
                        },
                        .value_v = IsNumber("1").with_span(3, 21, 3, 22)
                    }.with_name_span(3, 14, 3, 18)
                     .with_span(3, 5, 3, 22),
                    EnumCaseSpec{
                        .name = "Write",
                        .value_v = IsNumber("2").with_span(4, 13, 4, 14)
                    }.with_name_span(4, 5, 4, 10)
                     .with_span(4, 5, 4, 14)
                ).with_name_span(2, 6, 2, 16)
                 .with_span(1, 1, 5, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, TypeAliasDefinitionWithoutModifiers)
    {
        std::string code = "typealias IntList = list<int>";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .type_aliases = {
                IsTypeAlias("IntList",
                    /*modifiers=*/{},
                    /*target=*/IsType("list",
                        IsType("int").with_name_span(1, 26, 1, 29).with_span(1, 26, 1, 29)
                    ).with_name_span(1, 21, 1, 25).with_span(1, 21, 1, 30)
                ).with_name_span(1, 11, 1, 18)
                 .with_span(1, 1, 1, 30)
            }
        });
    }

    TEST_F(AstDeclSpanTest, TypeAliasDefinitionWithModifiers)
    {
        std::string code =
                "@deprecated(since: \"3.0\")\n"
                "typealias OldId = string";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .type_aliases = {
                IsTypeAlias("OldId",
                    /*modifiers=*/{
                        ModifierSpec{
                            .name = "deprecated",
                            .args = {
                                ArgSpec{.label = "since", .value_v = IsString("\"3.0\"").with_span(1, 20, 1, 25)}
                                    .with_name_span(1, 13, 1, 18)
                                    .with_span(1, 13, 1, 25)
                            }
                        }.with_name_span(1, 2, 1, 12)
                         .with_span(1, 1, 1, 26)
                    },
                    /*target=*/IsType("string").with_name_span(2, 19, 2, 25).with_span(2, 19, 2, 25)
                ).with_name_span(2, 11, 2, 16)
                 .with_span(1, 1, 2, 25)
            }
        });
    }

    TEST_F(AstDeclSpanTest, ExtensionDefinitionWithModifiers)
    {
        std::string code =
                "@category(name: \"MathExt\")\n"
                "extension int {\n"
                "    func square() -> int {\n"
                "        return 1\n"
                "    }\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .extensions = {
                IsExtensionDef(
                    /*modifiers=*/{
                        ModifierSpec{
                            .name = "category",
                            .args = {
                                ArgSpec{.label = "name", .value_v = IsString("\"MathExt\"").with_span(1, 17, 1, 26)}
                                    .with_name_span(1, 11, 1, 15)
                                    .with_span(1, 11, 1, 26)
                            }
                        }.with_name_span(1, 2, 1, 10)
                         .with_span(1, 1, 1, 27)
                    },
                    /*target=*/IsType("int").with_name_span(2, 11, 2, 14).with_span(2, 11, 2, 14),
                    /*spec=*/ProgramSpec{
                        .functions = {
                            IsFunctionDef("square",
                                /*modifiers=*/{},
                                /*params=*/{},
                                /*returns=*/{
                                    IsType("int").with_name_span(3, 22, 3, 25).with_span(3, 22, 3, 25)
                                },
                                /*body=*/{
                                    IsReturn(IsNumber("1").with_span(4, 16, 4, 17))
                                        .with_span(4, 9, 4, 17)
                                }
                            ).with_name_span(3, 10, 3, 16)
                             .with_span(3, 5, 5, 6)
                        }
                    }
                ).with_span(1, 1, 6, 2)
            }
        });
    }

    TEST_F(AstDeclSpanTest, EmptyDefinitions)
    {
        std::string code =
                "struct EmptyStruct {}\n"
                "enum EmptyEnum: int {}\n"
                "func emptyFunc() -> int {}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("emptyFunc",
                    /*modifiers=*/{},
                    /*params=*/{},
                    /*returns=*/{IsType("int").with_name_span(3, 21, 3, 24).with_span(3, 21, 3, 24)},
                    /*body=*/{}
                ).with_name_span(3, 6, 3, 15)
                 .with_span(3, 1, 3, 27)
            },
            .structs = {
                IsStructDef("EmptyStruct")
                    .with_name_span(1, 8, 1, 19)
                    .with_span(1, 1, 1, 22)
            },
            .enums = {
                IsEnumDef("EmptyEnum",
                    /*modifiers=*/{},
                    /*type=*/IsType("int").with_name_span(2, 17, 2, 20).with_span(2, 17, 2, 20),
                    /*cases=*/{}
                ).with_name_span(2, 6, 2, 15)
                 .with_span(2, 1, 2, 23)
            }
        });
    }

    TEST_F(AstDeclSpanTest, FunctionDefinitionWithDocstring)
    {
        std::string code =
                "func documented() -> int {\n"
                "    \"\"\"Computes something.\"\"\"\n"
                "    return 42\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("documented",
                    /*modifiers=*/{},
                    /*params=*/{},
                    /*returns=*/{IsType("int").with_name_span(1, 22, 1, 25).with_span(1, 22, 1, 25)},
                    /*body=*/{
                        IsReturn(IsNumber("42").with_span(3, 12, 3, 14))
                            .with_span(3, 5, 3, 14)
                    },
                    /*docstring=*/"\"\"\"Computes something.\"\"\""
                ).with_name_span(1, 6, 1, 16)
                 .with_span(1, 1, 4, 2)
            }
        });
        ASSERT_EQ(ast->function_definitions.size(), 1);
        EXPECT_TRUE(ast->function_definitions[0]->docstring.has_value());
        EXPECT_EQ(ast->function_definitions[0]->docstring.value(), "\"\"\"Computes something.\"\"\"");
    }
}
