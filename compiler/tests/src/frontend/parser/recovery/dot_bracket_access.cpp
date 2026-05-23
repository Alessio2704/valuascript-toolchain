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

            // reg("EmptyBracketMissingClosing", "arr[",
            //     {
            //         {E::UnmatchedBracketAfterTensorElements, 0, 0, 0, 0, true}
            //     },
            //     IsBracket(IsIdentifier("arr"), IsNull())
            // );

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

            // reg("WrongClosingBracketPreservesAll", "arr[1}",
            //     {
            //         {E::InvalidExpression, 1, 7, 1, 8}
            //     },
            //     IsBracket(IsIdentifier("arr"), IsNumber("1"))
            // );

            // reg("unmatchedClosingBracketPreservesSlice", "arr[1:2",
            //     {
            //         {E::UnmatchedBracketAfterTensorIndex, 0, 0, 0, 0, true}
            //     },
            //     IsBracket(IsIdentifier("arr"), IsBinary(TokenType::Colon,
            //                                             IsNumber("1"),
            //                                             IsNumber("2")
            //               )
            //     )
            // );

            reg("UnexpectedCommaInBracket", "arr[1, 2]",
                {
                    {E::UnexpectedCommaInBracketAccess, 1, 6, 1, 7}
                },
                IsBracket(IsIdentifier("arr"), IsNull())
            );

            // reg("MissingDotAccessPropertyNewLine", "obj.",
            //     {
            //         {E::ExpectedPropertyName, 1, 5, 1, 6}
            //     },
            //     IsDot(IsIdentifier("obj"), "<error>")
            // );

            reg("GarbageDotAccessProperty", "obj.*",
                {
                    {E::ExpectedPropertyName, 1, 5, 1, 6}
                },
                IsDot(IsIdentifier("obj"), "<error>")
            );

            return true;
        }();
    }
}
