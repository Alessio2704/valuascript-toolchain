#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("GarbageTokenRecoversToNextCase",
                "switch(v) {\n"
                "    garbage\n"
                "    case A -> 1\n"
                "}",
                {
                    {E::ExpectedCaseOrDefaultInsideSwitchBody, 2, 5, 2, 12}
                },
                IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            );

            reg("NumberAsCase",
                "switch (res) { case 1 -> 10 }",
                {
                    {E::ExpectedEnumCaseNameAfterCase, 1, 21, 1, 22}
                },
                IsSwitch(
                    IsIdentifier("res"), {
                        SwitchCaseSpec{{"<error>"}, IsNumber("10")}
                    }
                )
            );

            reg("StringAsCase",
                "switch (res) { case \"UP\" -> 10 }",
                {
                    {E::ExpectedEnumCaseNameAfterCase, 1, 21, 1, 25}
                },
                IsSwitch(
                    IsIdentifier("res"), {
                        SwitchCaseSpec{{"<error>"}, IsNumber("10")}
                    }
                )
            );

            reg("MissingArrowRecoversToDefault",
                "switch(v) {\n"
                "    case A 1\n"
                "    default -> 2\n"
                "}",
                {
                    {E::ExpectedRightArrowAfterSwitchCaseIdentifier, 2, 12, 2, 13}
                },
                IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A"}, IsNull()}
                    },
                    IsNumber("2")
                )
            );

            reg("MultipleDefaultsRecovers",
                "switch(v) {\n"
                "    default -> 1\n"
                "    default -> 2\n"
                "}",
                {
                    {E::MultipleDefaultCasesInSwitch, 3, 5, 3, 12}
                },
                IsSwitch(
                    IsIdentifier("v"), {}, IsNumber("2")
                )
            );

            reg("MissingCommaInCaseIdentifiers",
                "switch(v) {\n"
                "    case A B -> 1\n"
                "}",
                {
                    {E::ExpectedCommaBetweenCaseIdentifiers, 2, 12, 2, 13}
                },
                IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A", "B"}, IsNumber("1")}
                    }
                )
            );

            reg("DanglingArrowRightBeforeClosingBrace",
                "switch(v) {\n"
                "    case A -> \n"
                "}",
                {
                    {E::InvalidExpression, 2, 12, 2, 14}
                },
                IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A"}, IsNull()}
                    }
                )
            );

            reg("SwitchTargetMissingParentheses",
                "switch v {\n"
                "    case A -> 1\n"
                "}",
                {
                    {E::ExpectedLeftParenAfterSwitch, 1, 8, 1, 9}
                },
                IsSwitch(
                    IsNull(), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            );

            reg("SwitchTargetCompletelyEmptyParens",
                "switch() {\n"
                "    case A -> 1\n"
                "}",
                {
                    {E::InvalidExpression, 1, 8, 1, 9}
                },
                IsSwitch(
                    IsNull(), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            );

            reg("SwitchTargetGarbageBetweenParens",
                "switch( . ) {\n"
                "    case A -> 1\n"
                "}",
                {
                    {E::InvalidExpression, 1, 9, 1, 10}
                },
                IsSwitch(
                    IsNull(), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            );

            reg("EmptySwitchBodyWithGarbage",
                "switch(v) {\n"
                "    + - * /\n"
                "}",
                {
                    {E::ExpectedCaseOrDefaultInsideSwitchBody, 2, 5, 2, 6}
                },
                IsSwitch(IsIdentifier("v"), {})
            );

            reg("EmptySlotsInCaseCommaList",
                "switch(v) {\n"
                "    case A, , B -> 1\n"
                "}",
                {
                    {E::ExpectedEnumCaseNameAfterCase, 2, 13, 2, 14}
                },
                IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A", "<error>", "B"}, IsNumber("1")}
                    }
                )
            );

            return true;
        }();
    }
}
