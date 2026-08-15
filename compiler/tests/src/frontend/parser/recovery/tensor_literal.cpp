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
                .name = "TensorDoubleComma",
                .code = "[1,, 2]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNull(), IsNumber("2")
                )
            });

            reg({
                .name = "TensorTrailingCommaGarbage",
                .code = "[1, 2, , ]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNumber("2"), IsNull()
                )
            });

            reg({
                .name = "TensorEmptyElementsStress",
                .code = "[ , , 1, , ]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                },
                .verifier = IsTensor(
                    IsNull(), IsNull(), IsNumber("1"), IsNull()
                )
            });

            reg({
                .name = "EmptyGarbageElement",
                .code = "[*]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                },
                .verifier = IsTensor(
                    IsNull()
                )
            });

            reg({
                .name = "GarbageElement",
                .code = "[1, *, 3]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "InvalidExpressionAsFirst",
                .code = "[*, 1]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                },
                .verifier = IsTensor(
                    IsNull(), IsNumber("1")
                )
            });

            reg({
                .name = "InvalidExpressionAsSecond",
                .code = "[1, *]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNull()
                )
            });

            reg({
                .name = "TensorBrokenMultiline",
                .code = "[\n"
                "1,\n"
                "*,\n"
                "3\n"
                "]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 2}
                },
                .verifier = IsTensor(
                    IsNumber("1"),
                    IsNull(),
                    IsNumber("3")
                )
            });

            reg({
                .name = "TensorRegressionComparison",
                .code = "[ (x > 0), * ]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                },
                .verifier = IsTensor(
                    IsGrouping(
                        IsBinary(
                            TokenType::Greater,
                            IsIdentifier("x"),
                            IsNumber("0")
                        )
                    ),
                    IsNull()
                )
            });

            reg({
                .name = "TensorLiteralWithSliceSyntax",
                .code = "[1:2, 3 ]",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                },
                .verifier = IsTensor(
                    IsNumber("1")
                )
            });

            reg({
                .name = "UnclosedTensorLiteral",
                .code = "[1, 2, 3",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsTensor(IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsGrouping(IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprUnaryGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsUnary(TokenType::Minus, IsGrouping(IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsGrouping(IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsNumber("2"), IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            return true;
        }();
    }
}
