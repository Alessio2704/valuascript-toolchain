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
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprEnumCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightBraceAfterEnumBody, .line_start = 1, .column_start = 27, .line_end = 1, .column_end = 28}
                        },
                        .verifier = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 27, .line_end = 1, .column_end = 28}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "k2", .value_v = IsNumber("2")},
                            DictItemSpec{.key = "k3", .value_v = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 27, .line_end = 1, .column_end = 28}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 41, .line_end = 1, .column_end = 42}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 34, .line_end = 1, .column_end = 35}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "k2", .value_v = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33},
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33}
                        },
                        .verifier = IsGrouping(IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 29, .line_end = 1, .column_end = 30},
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 29, .line_end = 1, .column_end = 30}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33},
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33}
                        },
                        .verifier = IsTensor(IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 29, .line_end = 1, .column_end = 30},
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 29, .line_end = 1, .column_end = 30}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 38, .line_end = 1, .column_end = 39},
                            PErr{.code = E::ExpectedRightParenAfterArguments, .line_start = 1, .column_start = 38, .line_end = 1, .column_end = 39}
                        },
                        .verifier = IsCall(IsIdentifier("f"), ArgSpec{.label = "arg", .value_v = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})}),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::CaseOrDefaultMissingInSwitchAfterResult, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27},
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33},
                            PErr{.code = E::ExpectedRightParenAfterArguments, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33}
                        },
                        .verifier = IsCall(IsIdentifier("f"), ArgSpec{.label = "a", .value_v = IsNumber("1")}, ArgSpec{.label = "arg", .value_v = IsSwitch(IsIdentifier("v"), SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")})}),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 4, .column_start = 15, .line_end = 4, .column_end = 16}
                        },
                        .verifier = IsSwitch(IsNumber("1"), SwitchCaseSpec{
                            .labels = {"A"},
                            .result_v = IsSwitch(
                                IsIdentifier("v"),
                                std::vector<SwitchCaseSpec>{
                                    SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")},
                                    SwitchCaseSpec{.labels = {"B"}, .result_v = IsNumber("2")},
                                    SwitchCaseSpec{.labels = {"C"}, .result_v = IsNumber("3")}
                                },
                                IsNumber("4")
                            )
                        }),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 3, .column_start = 15, .line_end = 3, .column_end = 16}
                        },
                        .verifier = IsSwitch(IsNumber("1"),
                            SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")},
                            SwitchCaseSpec{
                                .labels = {"B"},
                                .result_v = IsSwitch(
                                    IsIdentifier("v"),
                                    std::vector<SwitchCaseSpec>{
                                        SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")},
                                        SwitchCaseSpec{.labels = {"C"}, .result_v = IsNumber("3")}
                                    },
                                    IsNumber("4")
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 2, .column_start = 15, .line_end = 2, .column_end = 16}
                        },
                        .verifier = IsSwitch(IsNumber("1"),
                            SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")},
                            SwitchCaseSpec{.labels = {"B"}, .result_v = IsNumber("2")},
                            SwitchCaseSpec{
                                .labels = {"C"},
                                .result_v = IsSwitch(
                                    IsIdentifier("v"),
                                    std::vector<SwitchCaseSpec>{
                                        SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                                    },
                                    IsNumber("4")
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightBraceAfterSwitchBody, .line_start = 2, .column_start = 15, .line_end = 2, .column_end = 16}
                        },
                        .verifier = IsSwitch(IsNumber("1"),
                            SwitchCaseSpec{
                                .labels = {"A"},
                                .result_v = IsSwitch(
                                    IsIdentifier("v"),
                                    std::vector<SwitchCaseSpec>{
                                        SwitchCaseSpec{.labels = {"A"}, .result_v = IsNumber("1")}
                                    },
                                    IsNumber("4")
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprGrouping, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprUnaryGrouping, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTupleEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprBracketAccessIndex, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsCallTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsDotTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsBracketTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsSliceTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCond, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprBinaryLhs, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprBinaryRhs, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprIfCond, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprIfThen, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprFuncDefDefault, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprModifierArg, .skip_after_depth_0 = true}
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            return true;
        }();
    }
}
