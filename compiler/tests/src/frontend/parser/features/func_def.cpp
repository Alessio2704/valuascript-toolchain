#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class FunctionDefinitionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(FunctionDefinitionSuccessPathTest, MinimalFunction)
    {
        ExpectValidFunctionDefinition(
            "func f() -> void {}",
            IsFunctionDef("f", {}, {}, {IsType("void")}, {})
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, MultipleParameters)
    {
        ExpectValidFunctionDefinition(
            "func add(a: int, b: int) -> int {}",
            IsFunctionDef("add", {}, {
                              ParamSpec{"a", {}, IsType("int")},
                              ParamSpec{"b", {}, IsType("int")}
                          }, {IsType("int")}, {}
            )
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, SingleDefaultParameterValue)
    {
        ExpectValidFunctionDefinition(
            "func f(a: int, b: bool = true) -> void {}",
            IsFunctionDef("f", {}, {
                              ParamSpec{"a", {}, IsType("int")},
                              ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                          }, {IsType("void")}, {}
            )
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, DefaultParameterValues)
    {
        ExpectValidFunctionDefinition(
            "func f(a: int = 1, b: bool = true) -> void {}",
            IsFunctionDef("f", {}, {
                              ParamSpec{"a", {}, IsType("int"), IsNumber("1")},
                              ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                          }, {IsType("void")}, {}
            )
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, MultipleReturnTypes)
    {
        ExpectValidFunctionDefinition(
            "func f() -> int, string {}",
            IsFunctionDef("f", {}, {}, {
                              IsType("int"),
                              IsType("string")
                          }, {}
            )
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, FunctionWithDocstring)
    {
        ExpectValidFunctionDefinition(
            "func f() -> void {\n"
            "  \"\"\"This is a docstring\"\"\"\n"
            "}",
            IsFunctionDef("f", {}, {}, {IsType("void")}, {}, "\"\"\"This is a docstring\"\"\"")
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, FunctionWithBodyStatements)
    {
        ExpectValidFunctionDefinition(
            "func f() -> void {\n"
            "  let x = 1\n"
            "  return x\n"
            "}",
            IsFunctionDef("f", {}, {}, {IsType("void")}, {
                              IsAssignment({}, {{"x"}}, IsNumber("1")),
                              IsReturn({IsIdentifier("x")})
                          }
            )
        );
    }

    TEST_F(FunctionDefinitionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidFunctionDefinition(
            "func long_function_name(\n"
            "  param_one: int,\n"
            "  param_two: string\n"
            ") -> \n"
            "  bool, \n"
            "  decimal \n"
            "{\n"
            "}",
            IsFunctionDef("long_function_name", {}, {
                              ParamSpec{"param_one", {}, IsType("int")},
                              ParamSpec{"param_two", {}, IsType("string")}
                          }, {
                              IsType("bool"),
                              IsType("decimal")
                          }, {}
            )
        );
    }
}
