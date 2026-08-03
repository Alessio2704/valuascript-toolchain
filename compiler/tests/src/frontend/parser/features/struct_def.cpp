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
                .verifier = IsStructDef("S")
            });

            reg({
                .name = "SingleField",
                .code = "struct S { f: int }",
                .verifier = IsStructDef("S",
                    FieldSpec{.name = "f", .type_v = IsType("int")}
                )
            });

            reg({
                .name = "MultipleFields",
                .code = "struct Point { x: float, y: float, z: float }",
                .verifier = IsStructDef("Point",
                    FieldSpec{.name = "x", .type_v = IsType("float")},
                    FieldSpec{.name = "y", .type_v = IsType("float")},
                    FieldSpec{.name = "z", .type_v = IsType("float")}
                )
            });

            reg({
                .name = "TrailingComma",
                .code = "struct S { f1: int, f2: bool, }",
                .verifier = IsStructDef("S",
                    FieldSpec{.name = "f1", .type_v = IsType("int")},
                    FieldSpec{.name = "f2", .type_v = IsType("bool")}
                )
            });

            reg({
                .name = "InterleavingModifiedFields",
                .code = "struct User { @id id: int, username: string, @optional bio: string }",
                .verifier = IsStructDef("User",
                    FieldSpec{.name = "id", .modifiers = {{"id"}}, .type_v = IsType("int")},
                    FieldSpec{.name = "username", .type_v = IsType("string")},
                    FieldSpec{.name = "bio", .modifiers = {{"optional"}}, .type_v = IsType("string")}
                )
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
                .verifier = IsStructDef("Config",
                    FieldSpec{.name = "secret", .modifiers = {{"internal"}}, .type_v = IsType("string")},
                    FieldSpec{.name = "version", .type_v = IsType("int")}
                )
            });

            return true;
        }();
    }

    TEST_P(StructDefinitionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidStructDefinition(entry.code, entry.verifier, entry.skip_contexts);
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
