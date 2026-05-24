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
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("Simple", "let a = 1", IsAssignment({{{}, "a"}}, IsNumber("1")));

            reg("UnderscoreAndNumbers", "let _a_1 = 1", IsAssignment({{{}, "_a_1"}}, IsNumber("1")));

            reg("KeywordInsideIdentifier", "let ifthenelse = 1", IsAssignment({{{}, "ifthenelse"}}, IsNumber("1")));

            reg("ExplicitType", "let a: int = 1", IsAssignment({{{}, "a", IsType("int")}}, IsNumber("1")));

            reg("MultiBasic2Vars", "let a, b = 1", IsAssignment({{{}, "a"}, {{}, "b"}}, IsNumber("1")));

            reg("MultiBasic5Vars", "let a, b, c, d, e = 1",
                IsAssignment({{{}, "a"}, {{}, "b"}, {{}, "c"}, {{}, "d"}, {{}, "e"}}, IsNumber("1")));

            reg("MultiTypeAtStart", "let a: string, b = 1",
                IsAssignment({{{}, "a", IsType("string")}, {{}, "b"}}, IsNumber("1")));

            reg("MultiTypeInMiddle", "let a, b: string, c = 1",
                IsAssignment({{{}, "a"}, {{}, "b", IsType("string")}, {{}, "c"}}, IsNumber("1")));

            reg("MultiTypeAtEnd", "let a, b: bool = 1",
                IsAssignment({{{}, "a"}, {{}, "b", IsType("bool")}}, IsNumber("1")));

            reg("MultiTypeEverywhere", "let a: int, b: int = 1",
                IsAssignment({{{}, "a", IsType("int")}, {{}, "b", IsType("int")}}, IsNumber("1")));

            reg("Multiline",
                "let \n"
                "  a, \n"
                "  b: int \n"
                "= 1",
                IsAssignment({{{}, "a"}, {{}, "b", IsType("int")}}, IsNumber("1")));

            return true;
        }();
    }

    TEST_P(AssignmentRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidAssignment(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Assignment,
        AssignmentRegistryRunner,
        testing::ValuesIn(ConstructRegistry::assignments()),
        [](const testing::TestParamInfo<RegistryEntry<AssignmentVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
