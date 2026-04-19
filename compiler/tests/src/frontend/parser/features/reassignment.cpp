#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ReassignmentSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ReassignmentSuccessPathTest, SimpleIdentifierTarget)
    {
        ExpectValidReassignment("a = 1", IsReassignment(IsIdentifier("a"), IsNumber("1")));
    }

    TEST_F(ReassignmentSuccessPathTest, DotAccessTarget)
    {
        ExpectValidReassignment("obj.prop = 1",
                                IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1")));
    }

    TEST_F(ReassignmentSuccessPathTest, BracketAccessTarget)
    {
        ExpectValidReassignment("arr[0] = 1",
                                IsReassignment(IsBracket(IsIdentifier("arr"), IsNumber("0")), IsNumber("1")));
    }

    TEST_F(ReassignmentSuccessPathTest, SelfDotTarget)
    {
        ExpectValidReassignment("self.field = 1",
                                IsReassignment(IsDot(IsSelf(), "field"), IsNumber("1")));
    }

    TEST_F(ReassignmentSuccessPathTest, CallResultDotTarget)
    {
        ExpectValidReassignment("get().val = 1",
                                IsReassignment(IsDot(IsCall(IsIdentifier("get"), {}), "val"), IsNumber("1")));
    }

    TEST_F(ReassignmentSuccessPathTest, MultilineFormatting)
    {
        ExpectValidReassignment(
            "obj \n"
            "  .prop \n"
            "  = \n"
            "  1",
            IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1"))
        );
    }
}
