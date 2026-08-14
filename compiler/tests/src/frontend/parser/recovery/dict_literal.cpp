#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "DictMissingClosingBrace",
                .code = "{x: 1, y: 2 ",
                .errors = {
                    PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                ),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprEnumCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightBraceAfterEnumBody, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                        ),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 29}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsDict(
                                DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                DictItemSpec{.key = "y", .value_v = IsNumber("2")},
                                DictItemSpec{.key = "k2", .value_v = IsNumber("2")},
                                DictItemSpec{.key = "k3", .value_v = IsNumber("3")}
                            )}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "k2", .value_v = IsDict(
                                DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                DictItemSpec{.key = "y", .value_v = IsNumber("2")},
                                DictItemSpec{.key = "k3", .value_v = IsNumber("3")}
                            )}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "k2", .value_v = IsNumber("2")},
                            DictItemSpec{.key = "k3", .value_v = IsDict(
                                DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                            )}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsDict(
                                DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                            )}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 24, .line_end = 1, .column_end = 25}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{
                                .label = "arg",
                                .value_v = IsDict(
                                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                    DictItemSpec{.key = "y", .value_v = IsNumber("2")},
                                    DictItemSpec{.key = "b", .value_v = IsNumber("2")},
                                    DictItemSpec{.key = "c", .value_v = IsNumber("3")}
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{.label = "a", .value_v = IsNumber("1")},
                            ArgSpec{
                                .label = "arg",
                                .value_v = IsDict(
                                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                    DictItemSpec{.key = "y", .value_v = IsNumber("2")},
                                    DictItemSpec{.key = "c", .value_v = IsNumber("3")}
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{.label = "a", .value_v = IsNumber("1")},
                            ArgSpec{.label = "b", .value_v = IsNumber("2")},
                            ArgSpec{
                                .label = "arg",
                                .value_v = IsDict(
                                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{
                                .label = "arg",
                                .value_v = IsDict(
                                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                                )
                            }
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                        ),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                        ),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                        ),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                        ),
                        .skip_after_depth_0 = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "DictMissingKey",
                .code = "{ : 1, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictMissingComma",
                .code = "{ x: 1 y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedCommaSeparatorInDictionaryLiteral, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictMissingExpressionValue",
                .code = "{ x: , y: 2 }",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictBrokenExpressionValues",
                .code = "{ x: *, y: *, z: 3 }",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNull()},
                    DictItemSpec{.key = "z", .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "GarbageBetweenPairs",
                .code = "{ x: 1, +, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictEmptyComma",
                .code = "{ , }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()}
                )
            });

            reg({
                .name = "DictEmptyGarbage",
                .code = "{ * }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()}
                )
            });

            reg({
                .name = "MissingBothKeyAndValue",
                .code = "{ :, :, z: 3}",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5},
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "z", .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "MissingBothKeyAndValueVertical",
                .code = "{\n"
                ":,\n"
                ":,\n"
                "z: 3\n}",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2},
                    PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 2, .line_end = 2, .column_end = 3},
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 2},
                    PErr{.code = E::InvalidExpression, .line_start = 3, .column_start = 2, .line_end = 3, .column_end = 3}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "z", .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "DoubleCommaBetweenPairs",
                .code = "{ x: 1,, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "x", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "DictKeyIsString",
                .code = "{ \"x\": 1 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNumber("1")}
                )
            });

            reg({
                .name = "DictKeyIsNumber",
                .code = "{ 1: 1 }",
                .errors = {
                    PErr{.code = E::ExpectedDictionaryKey, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNumber("1")}
                )
            });

            reg({
                .name = "DictMissingColon",
                .code = "{ x 1, y: 2 }",
                .errors = {
                    PErr{.code = E::ExpectedColonAfterDictionaryKey, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDict(
                    DictItemSpec{.key = "<error>", .value_v = IsNull()},
                    DictItemSpec{.key = "y", .value_v = IsNumber("2")}
                )
            });

            return true;
        }();
    }
}
