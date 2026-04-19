#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class SwitchExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(SwitchExpressionSuccessPathTest, SimpleSwitchWithOneCase)
    {
        ExpectValidExpression(
            "switch (x) { case A -> 1 }",
            IsSwitch(
                IsIdentifier("x"),
                {
                    SwitchCaseSpec{{"A"}, IsNumber("1")}
                }
            )
        );
    }

    TEST_F(SwitchExpressionSuccessPathTest, MultipleLabelsInOneCase)
    {
        ExpectValidExpression(
            "switch (x) { case A, B, C -> 1 }",
            IsSwitch(
                IsIdentifier("x"),
                {
                    SwitchCaseSpec{{"A", "B", "C"}, IsNumber("1")}
                }
            )
        );
    }

    TEST_F(SwitchExpressionSuccessPathTest, MultipleCases)
    {
        ExpectValidExpression(
            "switch (x) { case A -> 1 case B -> 2 }",
            IsSwitch(
                IsIdentifier("x"),
                {
                    SwitchCaseSpec{{"A"}, IsNumber("1")},
                    SwitchCaseSpec{{"B"}, IsNumber("2")}
                }
            )
        );
    }

    TEST_F(SwitchExpressionSuccessPathTest, DefaultCaseOnly)
    {
        ExpectValidExpression(
            "switch (x) { default -> 0 }",
            IsSwitch(
                IsIdentifier("x"),
                {},
                IsNumber("0")
            )
        );
    }

    TEST_F(SwitchExpressionSuccessPathTest, FullSwitchWithDefault)
    {
        ExpectValidExpression(
            "switch (x) { case A -> 1 case B -> 2 default -> 0 }",
            IsSwitch(
                IsIdentifier("x"),
                {
                    SwitchCaseSpec{{"A"}, IsNumber("1")},
                    SwitchCaseSpec{{"B"}, IsNumber("2")}
                },
                IsNumber("0")
            )
        );
    }

    TEST_F(SwitchExpressionSuccessPathTest, NestedSwitchExpressions)
    {
        ExpectValidExpression(
            "switch (x) { case A -> switch (y) { case B -> 1 default -> 2 } default -> 3 }",
            IsSwitch(
                IsIdentifier("x"),
                {
                    SwitchCaseSpec{
                        {"A"},
                        IsSwitch(
                            IsIdentifier("y"),
                            {
                                SwitchCaseSpec{{"B"}, IsNumber("1")}
                            },
                            IsNumber("2")
                        )
                    }
                },
                IsNumber("3")
            )
        );
    }

    TEST_F(SwitchExpressionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidExpression(
            "switch (state) {\n"
            "  case Active, Pending -> \"ok\"\n"
            "  case Error -> \"fail\"\n"
            "  default -> \"unknown\"\n"
            "}",
            IsSwitch(
                IsIdentifier("state"),
                {
                    SwitchCaseSpec{{"Active", "Pending"}, IsString("\"ok\"")},
                    SwitchCaseSpec{{"Error"}, IsString("\"fail\"")}
                },
                IsString("\"unknown\"")
            )
        );
    }
}
