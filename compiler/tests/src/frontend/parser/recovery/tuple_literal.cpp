#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"
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
                .name = "SingleElementTupleNotAllowed",
                .code = "(1,)",
                .errors = {
                    PErr{
                        .code = E::SingleElementTuplesNotAllowed, .line_start = 1, .column_start = 3, .line_end = 1,
                        .column_end = 4
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1")
                )
            });

            reg({
                .name = "TupleMissingClosingParen",
                .code = "(1, 2",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1,
                        .column_end = 6
                    }
                },
                .verifier = IsTuple(IsNumber("1"), IsNumber("2")),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsGrouping(IsTuple(IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprUnaryGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsUnary(TokenType::Minus, IsGrouping(IsTuple(IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsGrouping(IsTuple(IsNumber("1"), IsNumber("2"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTuple(
                            IsNumber("1"),
                            IsTuple(IsNumber("1"), IsNumber("2"), IsNumber("3"))
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsTuple(
                            IsNumber("1"),
                            IsNumber("2"),
                            IsTuple(IsNumber("1"), IsNumber("2"))
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsTensor(IsTuple(IsNumber("1"), IsNumber("2"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsTuple(IsNumber("1"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsTuple(IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsTensor(
                            IsTuple(IsNumber("1"), IsNumber("2"))
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))},
                            DictItemSpec{.key = "k2", .value_v = IsNumber("2")},
                            DictItemSpec{.key = "k3", .value_v = IsNumber("3")}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "k2", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))},
                            DictItemSpec{.key = "k3", .value_v = IsNumber("3")}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsNumber("1")},
                            DictItemSpec{.key = "k2", .value_v = IsNumber("2")},
                            DictItemSpec{.key = "k3", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsDict(
                            DictItemSpec{.key = "k1", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{.label = "arg", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))},
                            ArgSpec{.label = "b", .value_v = IsNumber("2")},
                            ArgSpec{.label = "c", .value_v = IsNumber("3")}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{.label = "a", .value_v = IsNumber("1")},
                            ArgSpec{.label = "arg", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))},
                            ArgSpec{.label = "c", .value_v = IsNumber("3")}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{.label = "a", .value_v = IsNumber("1")},
                            ArgSpec{.label = "b", .value_v = IsNumber("2")},
                            ArgSpec{.label = "arg", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsCall(
                            IsIdentifier("f"),
                            ArgSpec{.label = "arg", .value_v = IsTuple(IsNumber("1"), IsNumber("2"))}
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCond,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseStart,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseMiddle,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseEnd,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseSingle,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBinaryLhs,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsTuple(IsNumber("1"), IsNumber("2")), IsNumber("100"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBinaryRhs,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsNumber("2")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfCond,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfElse,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBracketAccessIndex,
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsCallTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterArguments, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsGrouping(IsCall(IsTuple(IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsDotTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsGrouping(IsDot(IsTuple(IsNumber("1"), IsNumber("2")), "prop")),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsBracketTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                        },
                        .verifier = IsGrouping(IsBracket(IsTuple(IsNumber("1"), IsNumber("2")), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsSliceTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = IsGrouping(IsBracket(IsTuple(IsNumber("1"), IsNumber("2")), IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "TrailingCommaInTuple",
                .code = "(1, 2,)",
                .errors = {
                    PErr{
                        .code = E::TrailingCommaInTuple, .line_start = 1, .column_start = 6, .line_end = 1,
                        .column_end = 7
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNumber("2")
                )
            });

            reg({
                .name = "TupleMissingExpression",
                .code = "(1, , , 3)",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    },
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "TupleGarbageBetweenElements",
                .code = "(1, *, *, 3)",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    },
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "EmptyTupleWithGarbage",
                .code = "(*)",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3
                    }
                },
                .verifier = IsGrouping(IsNull())
            });

            reg({
                .name = "TupleWithColonInsteadOfComma",
                .code = "(1, x: 2)",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5,
                        .line_end = 1, .column_end = 6
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsIdentifier("x")
                ),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprGrouping, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprUnaryGrouping, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTupleStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTupleMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTupleEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprModifierArg, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprBracketAccessIndex, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsCallTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsDotTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsBracketTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprAsSliceTarget, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCond, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprBinaryLhs, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprBinaryRhs, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprIfCond, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprIfThen, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprFuncDefDefault, .skip_after_depth_0 = true}
                }
            });

            reg({
                .name = "TupleMultilineRecovery",
                .code = "(\n"
                "  1,\n"
                "  *,\n"
                "  3\n"
                ")",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 3, .column_start = 3, .line_end = 3, .column_end = 4
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNull(),
                    IsNumber("3")
                )
            });

            reg({
                .name = "EmptyTupleWithCommaIsInvalid",
                .code = "(,)",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3
                    },
                    PErr{
                        .code = E::SingleElementTuplesNotAllowed, .line_start = 1, .column_start = 2, .line_end = 1,
                        .column_end = 3
                    }
                },
                .verifier = IsTuple(IsNull())
            });

            reg({
                .name = "MultipleNestedUnclosedTuples",
                .code = "(1, (2, (3, (4,",
                .errors = {
                    PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16},
                    PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16},
                    PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16},
                    PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16}
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsTuple(
                        IsNumber("2"),
                        IsTuple(
                            IsNumber("3"),
                            IsTuple(
                                IsNumber("4")
                            )
                        )
                    )
                ),
                .skip_contexts = ContextNames::all_nested_expressions()
            });

            return true;
        }();
    }
}
