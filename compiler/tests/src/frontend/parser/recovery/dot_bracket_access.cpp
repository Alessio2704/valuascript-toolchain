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
                .name = "EmptyBracket",
                .code = "arr[]",
                .errors = {
                    PErr{.code = E::EmptyBracketAccess, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNull())
            });

            reg({
                .name = "InvalidExpressionInBracket",
                .code = "arr[*]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNull())
            });

            reg({
                .name = "InvalidExpressionInBracketSlice",
                .code = "arr[1:*]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNull()))
            });

            reg({
                .name = "InvalidExpressionInBracketSliceLeft",
                .code = "arr[*:2]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNull(), IsNumber("2")))
            });

            reg({
                .name = "SliceTooManyColonsThreeBounds",
                .code = "arr[1:2:3]",
                .errors = {
                    PErr{.code = E::TooManyColonsInBracketSlice, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")), IsNumber("3")))
            });

            reg({
                .name = "SliceTooManyColonsDoubleColon",
                .code = "arr[::3]",
                .errors = {
                    PErr{.code = E::TooManyColonsInBracketSlice, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsBinary(TokenType::Colon, IsNull(), IsNull()), IsNumber("3")))
            });

            reg({
                .name = "SliceTooManyColonsFourBounds",
                .code = "arr[1:2:3:4]",
                .errors = {
                    PErr{.code = E::TooManyColonsInBracketSlice, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                    PErr{.code = E::TooManyColonsInBracketSlice, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                },
                .verifier = IsBracket(
                    IsIdentifier("arr"),
                    IsBinary(TokenType::Colon,
                        IsBinary(TokenType::Colon,
                            IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")),
                            IsNumber("3")
                        ),
                        IsNumber("4")
                    )
                )
            });

            reg({
                .name = "BracketAccessMissingClosingBracket",
                .code = "arr[0",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBracketAccessIndex,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = IsTensor(IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsTensor(IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0")),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsGrouping(IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprUnaryGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsUnary(TokenType::Minus, IsGrouping(IsBracket(IsIdentifier("arr"), IsNumber("0")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsGrouping(IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsNumber("2"), IsBracket(IsIdentifier("arr"), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all(),
            });

            reg({
                .name = "BracketSliceMissingClosingBracket",
                .code = "arr[1:2",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBracketAccessIndex,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsTensor(IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsNumber("2"), IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTensor(IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprCallArgSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCaseSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsGrouping(IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprUnaryGrouping,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsUnary(TokenType::Minus, IsGrouping(IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsGrouping(IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsTuple(IsNumber("1"), IsNumber("2"), IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all(),
            });

            reg({
                .name = "UnexpectedCommaInBracket",
                .code = "arr[1, 2]",
                .errors = {
                    PErr{.code = E::UnexpectedCommaInBracketAccess, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNumber("1"))
            });

            reg({
                .name = "GarbageDotAccessProperty",
                .code = "obj.*",
                .errors = {
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDot(IsIdentifier("obj"), "<error>")
            });

            reg({
                .name = "MissingDotAccessProperty",
                .code = "obj.",
                .errors = {
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDot(IsIdentifier("obj"), "<error>"),
                .context_overrides = {
                    {
                        .context_name = ContextNames::ExprSingleAssignment,
                        .accepted_sentinels = SentinelKinds::all()
                    }
                },
                .excluded_sentinels = {
                    SentinelKind::ExprStmt
                }
            });

            reg({
                .name = "ConsecutiveInvalidBracketAccess1",
                .code = "arr[*][*]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsBracket(
                    IsBracket(
                        IsIdentifier("arr"),
                        IsNull()
                    ),
                    IsNull()
                )
            });

            reg({
                .name = "ConsecutiveInvalidBracketAccess2",
                .code = "arr[*][1][*]",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                },
                .verifier = IsBracket(
                    IsBracket(
                        IsBracket(
                            IsIdentifier("arr"),
                            IsNull()
                        ),
                        IsNumber("1")
                    ),
                    IsNull()
                )
            });

            reg({
                .name = "ConsecutiveInvalidDotAccess1",
                .code = "arr.*.*",
                .errors = {
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsDot(
                    IsDot(
                        IsIdentifier("arr"),
                        "<error>"
                    ),
                    "<error>"
                )
            });

            reg({
                .name = "ConsecutiveInvalidDotAccess2",
                .code = "arr.*.a.*",
                .errors = {
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                },
                .verifier = IsDot(
                    IsDot(
                        IsDot(
                            IsIdentifier("arr"),
                            "<error>"
                        ),
                        "a"
                    ),
                    "<error>"
                )
            });

            reg({
                .name = "ConsecutiveInvalidInterleaved1",
                .code = "obj.*[*]",
                .errors = {
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBracket(
                    IsDot(
                        IsIdentifier("obj"),
                        "<error>"
                    ),
                    IsNull()
                )
            });

            reg({
                .name = "ConsecutiveInvalidInterleaved2",
                .code = "arr[*].*",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6},
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}
                },
                .verifier = IsDot(
                    IsBracket(
                        IsIdentifier("arr"),
                        IsNull()
                    ),
                    "<error>"
                )
            });

            reg({
                .name = "DoubleDotAccess",
                .code = "obj..a",
                .errors = {
                    PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsDot(
                    IsDot(
                        IsIdentifier("obj"),
                        "<error>"
                    ),
                    "a"
                )
            });

            return true;
        }();
    }
}
