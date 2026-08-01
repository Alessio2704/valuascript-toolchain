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
                    {E::EmptyBracketAccess, 1, 4, 1, 5}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNull())
            });

            reg({
                .name = "InvalidExpressionInBracket",
                .code = "arr[*]",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNull())
            });

            reg({
                .name = "MultipleColumnsInBracketSlice",
                .code = "arr[1:2:3]",
                .errors = {
                    {E::UnmatchedBracketAfterTensorIndex, 1, 7, 1, 8}
                },
                .verifier = IsBracket(
                    IsIdentifier("arr"),
                    IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))
                )
            });

            reg({
                .name = "UnexpectedCommaInBracket",
                .code = "arr[1, 2]",
                .errors = {
                    {E::UnexpectedCommaInBracketAccess, 1, 6, 1, 7}
                },
                .verifier = IsBracket(IsIdentifier("arr"), IsNumber("1"))
            });

            reg({
                .name = "GarbageDotAccessProperty",
                .code = "obj.*",
                .errors = {
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
                },
                .verifier = IsDot(IsIdentifier("obj"), "<error>")
            });

            reg({
                .name = "MissingDotAccessProperty",
                .code = "obj.",
                .errors = {
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
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
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 8, 1, 9}
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
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 11, 1, 12}
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
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                    {E::ExpectedPropertyName, 1, 7, 1, 8}
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
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                    {E::ExpectedPropertyName, 1, 9, 1, 10}
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
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 7, 1, 8}
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
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::ExpectedPropertyName, 1, 8, 1, 9}
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
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
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
