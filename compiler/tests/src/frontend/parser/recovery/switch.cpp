#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names_helpers.h"

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
                    PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 2, .column_start = 5, .line_end = 2, .column_end = 12}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "NumberAsCase",
                .code = "switch (res) { case 1 -> 10 }",
                .errors = {
                    PErr{.code = E::ExpectedEnumCaseNameAfterCase, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}
                },
                .verifier = IsSwitch(
                    IsIdentifier("res"),
                    SwitchCaseSpec{.labels = {"<error>"}, .result_v = IsNumber("10")}
                )
            });

            reg({
                .name = "StringAsCase",
                .code = "switch (res) { case \"UP\" -> 10 }",
                .errors = {
                    PErr{.code = E::ExpectedEnumCaseNameAfterCase, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 25}
                },
                .verifier = IsSwitch(
                    IsIdentifier("res"),
                    SwitchCaseSpec{.labels = {"<error>"}, .result_v = IsNumber("10")}
                )
            });

            reg({
                .name = "MissingArrowRecoversToDefault",
                .code = "switch(v) {\n"
                "    case A 1\n"
                "    default -> 2\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedRightArrowAfterSwitchCaseIdentifier, .line_start = 2, .column_start = 12, .line_end = 2, .column_end = 13}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"),
                    std::vector<SwitchCaseSpec>{SwitchCaseSpec{.labels = {"A"}, .result_v = IsNull()}},
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
                    PErr{.code = E::MultipleDefaultCasesInSwitch, .line_start = 3, .column_start = 5, .line_end = 3, .column_end = 12}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"), std::vector<SwitchCaseSpec>{}, IsNumber("2")
                )
            });

            reg({
                .name = "MissingCommaInCaseIdentifiers",
                .code = "switch(v) {\n"
                "    case A B -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedCommaBetweenCaseIdentifiers, .line_start = 2, .column_start = 12, .line_end = 2, .column_end = 13}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"),
                    SwitchCaseSpec{.labels = {"A", "B"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "DanglingArrowRightBeforeClosingBrace",
                .code = "switch(v) {\n"
                "    case A -> \n"
                "}",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 12, .line_end = 2, .column_end = 14}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNull()}
                )
            });

            reg({
                .name = "SwitchTargetMissingParentheses",
                .code = "switch v {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedLeftParenAfterSwitch, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsSwitch(
                    IsNull(),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "SwitchTargetMissingOpeningParenRecovers",
                .code = "switch 1 ) {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedLeftParenAfterSwitch, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsSwitch(
                    IsNumber("1"),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "SwitchTargetCompletelyEmptyParens",
                .code = "switch() {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsSwitch(
                    IsNull(),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "SwitchTargetGarbageBetweenParens",
                .code = "switch( . ) {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                },
                .verifier = IsSwitch(
                    IsNull(),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "EmptySwitchBodyWithGarbage",
                .code = "switch(v) {\n"
                "    + - * /\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 2, .column_start = 5, .line_end = 2, .column_end = 6}
                },
                .verifier = IsSwitch(IsIdentifier("v"), std::vector<SwitchCaseSpec>{})
            });

            reg({
                .name = "EmptySlotsInCaseCommaList",
                .code = "switch(v) {\n"
                "    case A, , B -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedEnumCaseNameAfterCase, .line_start = 2, .column_start = 13, .line_end = 2, .column_end = 14}
                },
                .verifier = IsSwitch(
                    IsIdentifier("v"),
                    SwitchCaseSpec{.labels = {"A", "<error>", "B"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "SwitchTargetUnclosedParenRecoversAtBrace",
                .code = "switch ( 1 {\n"
                "    case A -> 1\n"
                "}",
                .errors = {
                    PErr{.code = E::ExpectedRightParenAfterSwitchTarget, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                },
                .verifier = IsSwitch(
                    IsNull(),
                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                )
            });

            reg({
                .name = "SwitchMissingClosingBrace",
                .code = "switch (v) { case A -> 1 ",
                .errors = {
                    PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 24, .line_end = 1, .column_end = 25}
                },
                .verifier = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}),
                .skip_contexts = ContextNames::all_nested_swallowing_switch_contexts(),
                .accepted_sentinels = SentinelKinds::all()
            });

            return true;
        }();
    }
}
