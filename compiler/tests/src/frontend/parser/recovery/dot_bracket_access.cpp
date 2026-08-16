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
                .skip_contexts = ContextNames::all_nested_swallowing_bracket_contexts(),
                .accepted_sentinels = SentinelKinds::all(),
            });

            reg({
                .name = "BracketSliceMissingClosingBracket",
                .code = "arr[1:2",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                .skip_contexts = ContextNames::all_nested_swallowing_bracket_contexts(),
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
                .excluded_sentinels = { SentinelKind::ExprStmt },
                .accepted_sentinels = SentinelKinds::all(),
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
