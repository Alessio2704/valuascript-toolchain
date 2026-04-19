#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ReassignmentRegistryRunner : public ParserTestBase,
                                       public testing::WithParamInterface<RegistryEntry<ReassignmentVerifier>>
    {
    };

    namespace
    {
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("SimpleIdentifierTarget",
                "a = 1",
                IsReassignment(IsIdentifier("a"), IsNumber("1")));

            reg("DotAccessTarget",
                "obj.prop = 1",
                IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1")));

            reg("BracketAccessTarget",
                "arr[0] = 1",
                IsReassignment(IsBracket(IsIdentifier("arr"), IsNumber("0")), IsNumber("1")));

            reg("SelfDotTarget",
                "self.field = 1",
                IsReassignment(IsDot(IsSelf(), "field"), IsNumber("1")));

            reg("CallResultDotTarget",
                "get().val = 1",
                IsReassignment(IsDot(IsCall(IsIdentifier("get"), {}), "val"), IsNumber("1")));

            reg("MultilineFormatting",
                "obj \n"
                "  .prop \n"
                "  = \n"
                "  1",
                IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1")));

            return true;
        }();
    }

    TEST_P(ReassignmentRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidReassignment(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Reassignment,
        ReassignmentRegistryRunner,
        testing::ValuesIn(ConstructRegistry::reassignments()),
        [](const testing::TestParamInfo<RegistryEntry<ReassignmentVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
