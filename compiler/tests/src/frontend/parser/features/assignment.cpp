#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(AssignmentSuccessPathTest, SimpleAssignment)
    {
        ExpectValidAssignment("let a = 1", IsAssignment({}, {{"a"}}, IsNumber("1")));
    }

    TEST_F(AssignmentSuccessPathTest, IdentifierVariations)
    {
        ExpectValidAssignment("let _a_1 = 1", IsAssignment({}, {{"_a_1"}}, IsNumber("1")));
        ExpectValidAssignment("let ifthenelse = 1", IsAssignment({}, {{"ifthenelse"}}, IsNumber("1")));
    }

    TEST_F(AssignmentSuccessPathTest, ExplicitType)
    {
        ExpectValidAssignment("let a: int = 1",
                              IsAssignment({}, {{"a", IsType("int")}}, IsNumber("1"))
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignmentBasic)
    {
        ExpectValidAssignment("let a, b = 1", IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1")));
        ExpectValidAssignment("let a, b, c, d, e = 1",
                              IsAssignment({}, {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}}, IsNumber("1")));
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignmentWithTypeAnnotations)
    {
        ExpectValidAssignment("let a: string, b = 1",
                              IsAssignment({}, {{"a", IsType("string")}, {"b"}}, IsNumber("1")));

        ExpectValidAssignment("let a, b: string, c = 1",
                              IsAssignment({}, {{"a"}, {"b", IsType("string")}, {"c"}}, IsNumber("1")));

        ExpectValidAssignment("let a, b: bool = 1",
                              IsAssignment({}, {{"a"}, {"b", IsType("bool")}}, IsNumber("1")));

        ExpectValidAssignment("let a: int, b: int = 1",
                              IsAssignment({}, {{"a", IsType("int")}, {"b", IsType("int")}}, IsNumber("1")));
    }

    TEST_F(AssignmentSuccessPathTest, MultilineFormatting)
    {
        ExpectValidAssignment(
            "let \n"
            "  a, \n"
            "  b: int \n"
            "= 1",
            IsAssignment({}, {{"a"}, {"b", IsType("int")}}, IsNumber("1"))
        );
    }
}
