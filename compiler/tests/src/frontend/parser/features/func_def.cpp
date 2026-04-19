#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class FunctionDefinitionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(FunctionDefinitionSuccessPathTest, MinimalFunction)
    {
        ExpectValidParse(
            "func f() -> void {}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("f", {}, {}, {IsType("void")}, {})
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, MultipleParameters)
    {
        ExpectValidParse(
            "func add(a: int, b: int) -> int {}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("add", {}, {
                                      ParamSpec{"a", {}, IsType("int")},
                                      ParamSpec{"b", {}, IsType("int")}
                                  }, {IsType("int")}, {})
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, SingleDefaultParameterValue)
    {
        ExpectValidParse(
            "func f(a: int, b: bool = true) -> void {}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("f", {}, {
                                      ParamSpec{"a", {}, IsType("int")},
                                      ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                                  }, {IsType("void")}, {})
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, DefaultParameterValues)
    {
        ExpectValidParse(
            "func f(a: int = 1, b: bool = true) -> void {}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("f", {}, {
                                      ParamSpec{"a", {}, IsType("int"), IsNumber("1")},
                                      ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                                  }, {IsType("void")}, {})
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, MultipleReturnTypes)
    {
        ExpectValidParse(
            "func f() -> int, string {}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("f", {}, {}, {
                                      IsType("int"),
                                      IsType("string")
                                  }, {})
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, FunctionWithDocstring)
    {
        ExpectValidParse(
            "func f() -> void {\n"
            "  \"\"\"This is a docstring\"\"\"\n"
            "}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("f", {}, {}, {IsType("void")}, {}, "\"\"\"This is a docstring\"\"\"")
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, FunctionWithBodyStatements)
    {
        ExpectValidParse(
            "func f() -> void {\n"
            "  let x = 1\n"
            "  return x\n"
            "}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("f", {}, {}, {IsType("void")}, {
                                      IsAssignment({}, {{"x"}}, IsNumber("1")),
                                      IsReturn({IsIdentifier("x")})
                                  })
                }
            }
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidParse(
            "func long_function_name(\n"
            "  param_one: int,\n"
            "  param_two: string\n"
            ") -> \n"
            "  bool, \n"
            "  decimal \n"
            "{\n"
            "}",
            ProgramSpec{
                .functions = {
                    IsFunctionDef("long_function_name", {}, {
                                      ParamSpec{"param_one", {}, IsType("int")},
                                      ParamSpec{"param_two", {}, IsType("string")}
                                  }, {
                                      IsType("bool"),
                                      IsType("decimal")
                                  }, {})
                }
            }
        );
    }
}
