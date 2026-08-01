#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class StructDefinitionRegistryRunner : public ParserTestBase,
                                           public testing::WithParamInterface<RegistryEntry<StructVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<StructVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "MinimalStruct",
                .code = "struct S {}",
                .verifier = IsStructDef("S", {}, {})
            });

            reg({
                .name = "SingleField",
                .code = "struct S { f: int }",
                .verifier = IsStructDef("S", {}, {
                    FieldSpec{"f", {}, IsType("int")}
                })
            });

            reg({
                .name = "MultipleFields",
                .code = "struct Point { x: float, y: float, z: float }",
                .verifier = IsStructDef("Point", {}, {
                    FieldSpec{"x", {}, IsType("float")},
                    FieldSpec{"y", {}, IsType("float")},
                    FieldSpec{"z", {}, IsType("float")}
                })
            });

            reg({
                .name = "TrailingComma",
                .code = "struct S { f1: int, f2: bool, }",
                .verifier = IsStructDef("S", {}, {
                    FieldSpec{"f1", {}, IsType("int")},
                    FieldSpec{"f2", {}, IsType("bool")}
                })
            });

            reg({
                .name = "InterleavingModifiedFields",
                .code = "struct User { @id id: int, username: string, @optional bio: string }",
                .verifier = IsStructDef("User", {}, {
                    FieldSpec{"id", {{"id"}}, IsType("int")},
                    FieldSpec{"username", {}, IsType("string")},
                    FieldSpec{"bio", {{"optional"}}, IsType("string")}
                })
            });

            reg({
                .name = "MultilineFormatting",
                .code = "struct \n"
                "  Config \n"
                "{\n"
                "  @internal \n"
                "  secret: string, \n"
                "  \n"
                "  version: int \n"
                "}",
                .verifier = IsStructDef("Config", {}, {
                    FieldSpec{"secret", {{"internal"}}, IsType("string")},
                    FieldSpec{"version", {}, IsType("int")}
                })
            });

            return true;
        }();
    }

    TEST_P(StructDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidStructDefinition(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        StructDefinition,
        StructDefinitionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::structs()),
        [](const testing::TestParamInfo<RegistryEntry<StructVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
