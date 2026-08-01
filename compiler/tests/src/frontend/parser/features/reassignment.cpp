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
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ReassignmentVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "SimpleIdentifierTarget",
                .code = "a = 1",
                .verifier = IsReassignment(IsIdentifier("a"), IsNumber("1"))
            });

            reg({
                .name = "DotAccessTarget",
                .code = "obj.prop = 1",
                .verifier = IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1"))
            });

            reg({
                .name = "BracketAccessTarget",
                .code = "arr[0] = 1",
                .verifier = IsReassignment(IsBracket(IsIdentifier("arr"), IsNumber("0")), IsNumber("1"))
            });

            reg({
                .name = "SelfDotTarget",
                .code = "self.field = 1",
                .verifier = IsReassignment(IsDot(IsSelf(), "field"), IsNumber("1"))
            });

            reg({
                .name = "CallResultDotTarget",
                .code = "get().val = 1",
                .verifier = IsReassignment(IsDot(IsCall(IsIdentifier("get"), {}), "val"), IsNumber("1"))
            });

            reg({
                .name = "MultilineFormatting",
                .code = "obj \n"
                "  .prop \n"
                "  = \n"
                "  1",
                .verifier = IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1"))
            });

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
        [](const testing::TestParamInfo<RegistryEntry<ReassignmentVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
