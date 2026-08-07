#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class AssignmentRegistryRunner : public ParserTestBase,
                                     public testing::WithParamInterface<RegistryEntry<AssignmentVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<AssignmentVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "Simple",
                .code = "let a = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a"}}, IsNumber("1"))
            });

            reg({
                .name = "UnderscoreAndNumbers",
                .code = "let _a_1 = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="_a_1"}}, IsNumber("1"))
            });

            reg({
                .name = "KeywordInsideIdentifier",
                .code = "let ifthenelse = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="ifthenelse"}}, IsNumber("1"))
            });

            reg({
                .name = "ExplicitType",
                .code = "let a: int = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a", .type_v=IsType("int")}}, IsNumber("1"))
            });

            reg({
                .name = "MultiBasic2Vars",
                .code = "let a, b = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a"}, {.modifiers={}, .name="b"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiBasic5Vars",
                .code = "let a, b, c, d, e = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a"}, {.modifiers={}, .name="b"}, {.modifiers={}, .name="c"}, {.modifiers={}, .name="d"}, {.modifiers={}, .name="e"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiTypeAtStart",
                .code = "let a: string, b = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a", .type_v=IsType("string")}, {.modifiers={}, .name="b"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiTypeInMiddle",
                .code = "let a, b: string, c = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a"}, {.modifiers={}, .name="b", .type_v=IsType("string")}, {.modifiers={}, .name="c"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiTypeAtEnd",
                .code = "let a, b: bool = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a"}, {.modifiers={}, .name="b", .type_v=IsType("bool")}}, IsNumber("1"))
            });

            reg({
                .name = "MultiTypeEverywhere",
                .code = "let a: int, b: int = 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a", .type_v=IsType("int")}, {.modifiers={}, .name="b", .type_v=IsType("int")}}, IsNumber("1"))
            });

            reg({
                .name = "Multiline",
                .code = "let \n"
                "  a, \n"
                "  b: int \n"
                "= 1",
                .verifier = IsAssignment({{.modifiers={}, .name="a"}, {.modifiers={}, .name="b", .type_v=IsType("int")}}, IsNumber("1"))
            });

            return true;
        }();
    }

    TEST_P(AssignmentRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidAssignment(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        Assignment,
        AssignmentRegistryRunner,
        testing::ValuesIn(ConstructRegistry::assignments()),
        TestNameGenerator{}
    );
}
