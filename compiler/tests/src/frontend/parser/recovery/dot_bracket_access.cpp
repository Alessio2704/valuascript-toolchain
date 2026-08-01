#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](
                auto n,
                auto c,
                const std::vector<ParserExpectedError>& errs,
                const OneOf<ExprVerifier>& v,
                const std::vector<std::string_view>& skip_contexts = {},
                const std::vector<ContextOverride<ExprVerifier>>& context_overrides = {},
                const std::vector<SentinelKind>& excluded_sentinels = {},
                const std::vector<SentinelKind>& accepted_sentinels = {}
            )
            {
                ErrorRegistry::add(n, c, errs, v, skip_contexts, context_overrides, excluded_sentinels,
                                   accepted_sentinels);
            };

            reg("EmptyBracket", "arr[]",
                {
                    {E::EmptyBracketAccess, 1, 4, 1, 5}
                },
                IsBracket(IsIdentifier("arr"), IsNull())
            );

            reg("InvalidExpressionInBracket", "arr[*]",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsBracket(IsIdentifier("arr"), IsNull())
            );

            // reg("InvalidRightExpressionInBracketSlice", "arr[1 : *]",
            //     {
            //         {E::InvalidExpression, 1, 9, 1, 10}
            //     },
            //     IsBracket(IsIdentifier("arr"),
            //               IsBinary(TokenType::Colon,
            //                        IsNumber("1"),
            //                        IsNull()
            //               )
            //     )
            // );

            reg("MultipleColumnsInBracketSlice", "arr[1:2:3]",
                {
                    {E::UnmatchedBracketAfterTensorIndex, 1, 7, 1, 8}
                },
                IsBracket(IsIdentifier("arr"),
                          IsBinary(TokenType::Colon,
                                   IsNumber("1"),
                                   IsNumber("2")
                          )
                )
            );

            reg("UnexpectedCommaInBracket", "arr[1, 2]",
                {
                    {E::UnexpectedCommaInBracketAccess, 1, 6, 1, 7}
                },
                IsBracket(IsIdentifier("arr"), IsNumber("1"))
            );

            reg("GarbageDotAccessProperty", "obj.*",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
                },
                IsDot(IsIdentifier("obj"), "<error>")
            );

            reg("MissingDotAccessProperty", "obj.",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
                },
                IsDot(IsIdentifier("obj"), "<error>"),
                {},
                {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSingleAssignment,
                        .accepted_sentinels = SentinelKinds::all()
                    }
                },
                {
                    SentinelKind::ExprStmt
                }
            );

            reg("ConsecutiveInvalidBracketAccess1", "arr[*][*]",
                {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 8, 1, 9},
                },
                IsBracket(
                    IsBracket(
                        IsIdentifier("arr"),
                        IsNull()
                    ),
                    IsNull()
                )
            );

            reg("ConsecutiveInvalidBracketAccess2", "arr[*][1][*]",
                {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 11, 1, 12},
                },
                IsBracket(
                    IsBracket(
                        IsBracket(
                            IsIdentifier("arr"),
                            IsNull()
                        ),
                        IsNumber("1")
                    ),
                    IsNull()
                )
            );

            reg("ConsecutiveInvalidDotAccess1", "arr.*.*",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                    {E::ExpectedPropertyName, 1, 7, 1, 8},
                },
                IsDot(
                    IsDot(
                        IsIdentifier("arr"),
                        "<error>"
                    ),
                    "<error>"
                )
            );

            reg("ConsecutiveInvalidDotAccess2", "arr.*.a.*",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                    {E::ExpectedPropertyName, 1, 9, 1, 10},
                },
                IsDot(
                    IsDot(
                        IsDot(
                            IsIdentifier("arr"),
                            "<error>"
                        ),
                        "a"
                    ),
                    "<error>"
                )
            );

            reg("ConsecutiveInvalidInterleaved1", "obj.*[*]",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 7, 1, 8},
                },
                IsBracket(
                    IsDot(
                        IsIdentifier("obj"),
                        "<error>"
                    ),
                    IsNull()
                )
            );

            reg("ConsecutiveInvalidInterleaved2", "arr[*].*",
                {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::ExpectedPropertyName, 1, 8, 1, 9},
                },
                IsDot(
                    IsBracket(
                        IsIdentifier("arr"),
                        IsNull()
                    ),
                    "<error>"
                )
            );

            reg("DoubleDotAccess", "obj..a",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6},
                },
                IsDot(
                    IsDot(
                        IsIdentifier("obj"),
                        "<error>"
                    ),
                    "a"
                )
            );

            return true;
        }();
    }
}
