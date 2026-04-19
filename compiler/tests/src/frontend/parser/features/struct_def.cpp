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
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("MinimalStruct",
                "struct S {}",
                IsStructDef("S", {}, {}));

            reg("SingleField",
                "struct S { f: int }",
                IsStructDef("S", {}, {
                                FieldSpec{"f", {}, IsType("int")}
                            }));

            reg("MultipleFields",
                "struct Point { x: float, y: float, z: float }",
                IsStructDef("Point", {}, {
                                FieldSpec{"x", {}, IsType("float")},
                                FieldSpec{"y", {}, IsType("float")},
                                FieldSpec{"z", {}, IsType("float")}
                            }));

            reg("TrailingComma",
                "struct S { f1: int, f2: bool, }",
                IsStructDef("S", {}, {
                                FieldSpec{"f1", {}, IsType("int")},
                                FieldSpec{"f2", {}, IsType("bool")}
                            }));

            reg("InterleavingModifiedFields",
                "struct User { @id id: int, username: string, @optional bio: string }",
                IsStructDef("User", {}, {
                                FieldSpec{"id", {{"id"}}, IsType("int")},
                                FieldSpec{"username", {}, IsType("string")},
                                FieldSpec{"bio", {{"optional"}}, IsType("string")}
                            }));

            reg("MultilineFormatting",
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
                            }));

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
        [](const testing::TestParamInfo<RegistryEntry<StructVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
