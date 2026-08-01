#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "GarbageTokenRecoversToNextCase",
                .code = "switch(v) {\n"
                "    garbage\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    {E::ExpectedCaseOrDefaultInsideSwitchBody, 2, 5, 2, 12}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "NumberAsCase",
                .code = "switch (res) { case 1 -> 10 }",
                .errors = {
                    {E::ExpectedEnumCaseNameAfterCase, 1, 21, 1, 22}
                },
                .verifier = IsSwitch(
                    IsIdentifier("res"), {
                        SwitchCaseSpec{{"<error>"}, IsNumber("10")}
                    }
                )
            });

            reg({
                .name = "StringAsCase",
                .code = "switch (res) { case \"UP\" -> 10 }",
                .errors = {
                    {E::ExpectedEnumCaseNameAfterCase, 1, 21, 1, 25}
                },
                .verifier = IsSwitch(
                    IsIdentifier("res"), {
                        SwitchCaseSpec{{"<error>"}, IsNumber("10")}
                    }
                )
            });

            reg({
                .name = "MissingArrowRecoversToDefault",
                .code = "switch(v) {\n"
                "    case A 1\n"
                "    default -> 2\n"
                "}",
                .errors = {
                    {E::ExpectedRightArrowAfterSwitchCaseIdentifier, 2, 12, 2, 13}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A"}, IsNull()}
                    },
                    IsNumber("2")
                )
            });

            reg({
                .name = "MultipleDefaultsRecovers",
                .code = "switch(v) {\n"
                "    default -> 1\n"
                "    default -> 2\n"
                "}",
                .errors = {
                    {E::MultipleDefaultCasesInSwitch, 3, 5, 3, 12}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), {}, IsNumber("2")
                )
            });

            reg({
                .name = "MissingCommaInCaseIdentifiers",
                .code = "switch(v) {\n"
                "    case A B -> 1\n"
                "}",
                .errors = {
                    {E::ExpectedCommaBetweenCaseIdentifiers, 2, 12, 2, 13}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A", "B"}, IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "DanglingArrowRightBeforeClosingBrace",
                .code = "switch(v) {\n"
                "    case A -> \n"
                "}",
                .errors = {
                    {E::InvalidExpression, 2, 12, 2, 14}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A"}, IsNull()}
                    }
                )
            });

            reg({
                .name = "SwitchTargetMissingParentheses",
                .code = "switch v {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    {E::ExpectedLeftParenAfterSwitch, 1, 8, 1, 9}
                },
                .verifier = IsSwitch(
                    IsNull(), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "SwitchTargetCompletelyEmptyParens",
                .code = "switch() {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    {E::InvalidExpression, 1, 8, 1, 9}
                },
                .verifier = IsSwitch(
                    IsNull(), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "SwitchTargetGarbageBetweenParens",
                .code = "switch( . ) {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    {E::InvalidExpression, 1, 9, 1, 10}
                },
                .verifier = IsSwitch(
                    IsNull(), {
                        SwitchCaseSpec{{"A"}, IsNumber("1")}
                    }
                )
            });

            reg({
                .name = "EmptySwitchBodyWithGarbage",
                .code = "switch(v) {\n"
                "    + - * /\n"
                "}",
                .errors = {
                    {E::ExpectedCaseOrDefaultInsideSwitchBody, 2, 5, 2, 6}
                },
                .verifier = IsSwitch(IsIdentifier("v"), {})
            });

            reg({
                .name = "EmptySlotsInCaseCommaList",
                .code = "switch(v) {\n"
                "    case A, , B -> 1\n"
                "}",
                .errors = {
                    {E::ExpectedEnumCaseNameAfterCase, 2, 13, 2, 14}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), {
                        SwitchCaseSpec{{"A", "<error>", "B"}, IsNumber("1")}
                    }
                )
            });

            return true;
        }();
    }
}
