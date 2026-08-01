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
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<FuncVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "MinimalFunction",
                .code = "func f() -> void {}",
                .verifier = IsFunctionDef("f", {}, {}, {IsType("void")}, {})
            });

            reg({
                .name = "MultipleParameters",
                .code = "func add(a: int, b: int) -> int {}",
                .verifier = IsFunctionDef("add", {}, {
                    ParamSpec{"a", {}, IsType("int")},
                    ParamSpec{"b", {}, IsType("int")}
                }, {IsType("int")}, {})
            });

            reg({
                .name = "SingleDefaultParameterValue",
                .code = "func f(a: int, b: bool = true) -> void {}",
                .verifier = IsFunctionDef("f", {}, {
                    ParamSpec{"a", {}, IsType("int")},
                    ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                }, {IsType("void")}, {})
            });

            reg({
                .name = "DefaultParameterValues",
                .code = "func f(a: int = 1, b: bool = true) -> void {}",
                .verifier = IsFunctionDef("f", {}, {
                    ParamSpec{"a", {}, IsType("int"), IsNumber("1")},
                    ParamSpec{"b", {}, IsType("bool"), IsBoolean(true)}
                }, {IsType("void")}, {})
            });

            reg({
                .name = "MultipleReturnTypes",
                .code = "func f() -> int, string {}",
                .verifier = IsFunctionDef("f", {}, {}, {
                    IsType("int"),
                    IsType("string")
                }, {})
            });

            reg({
                .name = "FunctionWithDocstring",
                .code = "func f() -> void {\n"
                "  \"\"\"This is a docstring\"\"\"\n"
                "}",
                .verifier = IsFunctionDef("f", {}, {}, {IsType("void")}, {}, "\"\"\"This is a docstring\"\"\"")
            });

            reg({
                .name = "FunctionWithBodyStatements",
                .code = "func f() -> void {\n"
                "  let x = 1\n"
                "  return x\n"
                "}",
                .verifier = IsFunctionDef("f", {}, {}, {IsType("void")}, {
                    IsAssignment({{{}, "x"}}, IsNumber("1")),
                    IsReturn({}, {IsIdentifier("x")})
                })
            });

            reg({
                .name = "MultilineFormatting",
                .code = "func long_function_name(\n"
                "  param_one: int,\n"
                "  param_two: string\n"
                ") -> \n"
                "  bool, \n"
                "  decimal \n"
                "{\n"
                "}",
                .verifier = IsFunctionDef("long_function_name", {}, {
                    ParamSpec{"param_one", {}, IsType("int")},
                    ParamSpec{"param_two", {}, IsType("string")}
                }, {
                    IsType("bool"),
                    IsType("decimal")
                }, {})
            });

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
        [](const testing::TestParamInfo<RegistryEntry<FuncVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
