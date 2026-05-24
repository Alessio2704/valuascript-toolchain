#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
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
                    {E::UnmatchedBracketAfterTensorIndex, 1, 8, 1, 9}
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
                IsBracket(IsIdentifier("arr"), IsNull())
            );

            reg("GarbageDotAccessProperty", "obj.*",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
                },
                IsDot(IsIdentifier("obj"), "<error>")
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
