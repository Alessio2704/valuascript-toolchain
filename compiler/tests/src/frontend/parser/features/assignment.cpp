#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(AssignmentSuccessPathTest, Underscore)
    {
        ExpectValidParse(
            "let _a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"_a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, UnderscoreAndNumber)
    {
        ExpectValidParse(
            "let _a_1 = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"_a_1"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, IdentifierContainingKeyword)
    {
        ExpectValidParse(
            "let ifthenelse = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"ifthenelse"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ExplicitTypeSimple)
    {
        ExpectValidParse(
            "let a: integer = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", IsType("integer")}}, IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignment2Vars)
    {
        ExpectValidParse(
            "let a, b = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a"},
                                     {"b"}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignment3Vars)
    {
        ExpectValidParse(
            "let a, b, c = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a"},
                                     {"b"},
                                     {"c"}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignment4Vars)
    {
        ExpectValidParse(
            "let a, b, c, d = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a"},
                                     {"b"},
                                     {"c"},
                                     {"d"}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignment5Vars)
    {
        ExpectValidParse(
            "let a, b, c, d, e = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a"},
                                     {"b"},
                                     {"c"},
                                     {"d"},
                                     {"e"}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignmentTypeAll)
    {
        ExpectValidParse(
            "let a: int, b: int = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a", IsType("int")},
                                     {"b", IsType("int")}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignmentTypeStart)
    {
        ExpectValidParse(
            "let a: string, b = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a", IsType("string")},
                                     {"b"}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignmentTypeMid)
    {
        ExpectValidParse(
            "let a, b: string, c = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a"},
                                     {"b", IsType("string")},
                                     {"c"}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignmentTypeEnd)
    {
        ExpectValidParse(
            "let a, b: bool = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a"},
                                     {"b", IsType("bool")}
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }
}
