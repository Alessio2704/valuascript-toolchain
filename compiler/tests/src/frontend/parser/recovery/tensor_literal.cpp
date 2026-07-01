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

            reg("TensorDoubleComma", "[1,, 2]",
                {
                    {E::InvalidExpression, 1, 4, 1, 5}
                },
                IsTensor({
                    IsNumber("1"), IsNull(), IsNumber("2")
                })
            );

            reg("TensorTrailingCommaGarbage", "[1, 2, , ]",
                {
                    {E::InvalidExpression, 1, 8, 1, 9}
                },
                IsTensor({
                    IsNumber("1"), IsNumber("2"), IsNull()
                })
            );

            reg("TensorEmptyElementsStress", "[ , , 1, , ]",
                {
                    {E::InvalidExpression, 1, 3, 1, 4},
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 10, 1, 11}
                },
                IsTensor({
                    IsNull(), IsNull(), IsNumber("1"), IsNull()
                })
            );

            reg("TensorMissingComma", "[1 2]",
                {
                    {E::MissingOperator, 1, 4, 1, 5}
                },
                IsTensor({
                    IsNumber("1"), IsNumber("2")
                })
            );

            reg("EmptyGarbageElement", "[*]",
                {
                    {E::InvalidExpression, 1, 2, 1, 3}
                },
                IsTensor({
                    IsNull()
                })
            );

            reg("GarbageElement", "[1, *, 3]",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsTensor({
                    IsNumber("1"), IsNull(), IsNumber("3")
                })
            );

            reg("InvalidExpressionAsFirst", "[*, 1]",
                {
                    {E::InvalidExpression, 1, 2, 1, 3}
                },
                IsTensor({
                    IsNull(), IsNumber("1")
                })
            );

            reg("InvalidExpressionAsSecond", "[1, *]",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsTensor({
                    IsNumber("1"), IsNull()
                })
            );

            reg("TensorBrokenMultiline",
                "[\n"
                "1,\n"
                "*,\n"
                "3\n"
                "]",
                {
                    {E::InvalidExpression, 3, 1, 3, 2}
                },
                IsTensor({
                    IsNumber("1"),
                    IsNull(),
                    IsNumber("3")
                })
            );

            reg("TensorRegressionComparison",
                "[ (x > 0), * ]",
                {
                    {E::InvalidExpression, 1, 12, 1, 13}
                },
                IsTensor({
                    IsGrouping(
                        IsBinary(
                            TokenType::Greater,
                            IsIdentifier("x"),
                            IsNumber("0")
                        )
                    ),
                    IsNull()
                })
            );

            // reg("TensorWithMismatchedNestedCloser", "[ ( 1 + 2 ], 3 ]",
            //     {
            //         {E::ExpectedRightParenAfterExpression, 1, 11, 1, 12}
            //     },
            //     IsTensor({
            //         IsGrouping(
            //             IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))
            //         ),
            //         IsNumber("3")
            //     })
            // );

            reg("TensorLiteralWithSliceSyntax", "[1:2, 3 ]",
                {
                    {E::UnmatchedBracketAfterTensorElements, 1, 3, 1, 4}
                },
                IsTensor({
                    IsNumber("1")
                })
            );

            return true;
        }();
    }
}
