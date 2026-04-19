#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class FunctionDefinitionRegistryRunner : public ParserTestBase,
                                             public testing::WithParamInterface<RegistryEntry<FuncVerifier>>
    {
    };

    namespace
    {
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("MinimalFunction",
                "func f() -> void {}",
                IsFunctionDef("f", {}, {}, {IsType("void")}, {}));

            reg("MultipleParameters",
                "func add(a: int, b: int) -> int {}",
                IsFunctionDef("add", {}, {
                                  ParamSpec{"a", {}, IsType("int")},
                                  ParamSpec{"b", {}, IsType("int")}
                              }, {IsType("int")}, {}));

            reg("SingleDefaultParameterValue",
                "func f(a: int, b: bool = true) -> void {}",
                IsFunctionDef("f", {}, {
                                  ParamSpec{"a", {}, IsType("int")},
                                  ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                              }, {IsType("void")}, {}));

            reg("DefaultParameterValues",
                "func f(a: int = 1, b: bool = true) -> void {}",
                IsFunctionDef("f", {}, {
                                  ParamSpec{"a", {}, IsType("int"), IsNumber("1")},
                                  ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                              }, {IsType("void")}, {}));

            reg("MultipleReturnTypes",
                "func f() -> int, string {}",
                IsFunctionDef("f", {}, {}, {
                                  IsType("int"),
                                  IsType("string")
                              }, {}));

            reg("FunctionWithDocstring",
                "func f() -> void {\n"
                "  \"\"\"This is a docstring\"\"\"\n"
                "}",
                IsFunctionDef("f", {}, {}, {IsType("void")}, {}, "\"\"\"This is a docstring\"\"\""));

            reg("FunctionWithBodyStatements",
                "func f() -> void {\n"
                "  let x = 1\n"
                "  return x\n"
                "}",
                IsFunctionDef("f", {}, {}, {IsType("void")}, {
                                  IsAssignment({}, {{"x"}}, IsNumber("1")),
                                  IsReturn({IsIdentifier("x")})
                              }));

            reg("MultilineFormatting",
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
                              }, {}));

            return true;
        }();
    }

    TEST_P(FunctionDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidFunctionDefinition(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionDefinition,
        FunctionDefinitionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::functions()),
        [](const testing::TestParamInfo<RegistryEntry<FuncVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
