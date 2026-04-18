#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(AssignmentSuccessPathTest, NumberLiteral)
    {
        ExpectValidParse(
            "let a = 1000",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1000"))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, PercentageLiteral1)
    {
        ExpectValidParse(
            "let a = 1.5%",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsPercentage("1.5%"))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, PercentageLiteral2)
    {
        ExpectValidParse(
            "let a = 0.000_001%",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsPercentage("0.000_001%"))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, StringLiteral)
    {
        ExpectValidParse(
            "let a = \"string\"",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsString("\"string\""))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, BooleanTrue)
    {
        ExpectValidParse(
            "let a = true",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsBoolean(true))
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, BooleanFalse)
    {
        ExpectValidParse(
            "let _a = false",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"_a"}}, IsBoolean(false))
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

    TEST_F(AssignmentSuccessPathTest, ExplicitTypeTuple)
    {
        ExpectValidParse(
            "let a: (integer, string) = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {
                                         "a",
                                         IsTupleType({
                                             IsType("integer"),
                                             IsType("string")
                                         })
                                     }
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ExplicitTypeTupleGeneric)
    {
        ExpectValidParse(
            "let a: (vector<integer>, string) = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {
                                         "a",
                                         IsTupleType({
                                             IsType("vector", {
                                                        IsType("integer")
                                                    }),
                                             IsType("string")
                                         })
                                     }
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ExplicitTypeGeneric)
    {
        ExpectValidParse(
            "let a: vector<integer> = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {
                                         "a",
                                         IsType("vector", {
                                                    IsType("integer")
                                                })
                                     }
                                 },
                                 IsNumber("1")
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

    TEST_F(AssignmentSuccessPathTest, Modifier1)
    {
        ExpectValidParse(
            "@export let a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"export"}},
                        {{"a"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, Modifier2)
    {
        ExpectValidParse(
            "@export @memoize let a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"export"}, {"memoize"}},
                        {{"a"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, Modifier3)
    {
        ExpectValidParse(
            "@mod1 @mod2 @mod3 let a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"mod1"}, {"mod2"}, {"mod3"}},
                        {{"a"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ModifierWithArgs)
    {
        ExpectValidParse(
            "@bind(target: \"ui\") let a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {
                                "bind", {
                                    {"target", IsString("\"ui\"")}
                                }
                            }
                        },
                        {{"a"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ModifierMixedArgsAndNoArgs)
    {
        ExpectValidParse(
            "@export @bind(target: \"ui\") @safe let a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"export"},
                            {
                                "bind", {
                                    {"target", IsString("\"ui\"")}
                                }
                            },
                            {"safe"}
                        },
                        {{"a"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ModifierAcrossNewlines)
    {
        ExpectValidParse(
            "@export\n@safe\nlet a = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"export"}, {"safe"}},
                        {{"a"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ModifiersWithMultiAssignment)
    {
        ExpectValidParse(
            "@export let a, b = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"export"}},
                        {{"a"}, {"b"}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, ModifiersWithTypes)
    {
        ExpectValidParse(
            "@export let a: int = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"export"}},
                        {{"a", IsType("int")}},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(AssignmentSuccessPathTest, MultiAssignemntTypesAndComplexModifiers)
    {
        ExpectValidParse(
            "@export @meta(id: 10) let a: int, b, c: string = 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"export"},
                            {
                                "meta", {
                                    {"id", IsNumber("10")}
                                }
                            }
                        },
                        {
                            {"a", IsType("int")},
                            {"b"},
                            {"c", IsType("string")}
                        },
                        IsNumber("1")
                    )
                }
            }
        );
    }
}
