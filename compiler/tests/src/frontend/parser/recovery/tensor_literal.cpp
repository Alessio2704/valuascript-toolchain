#include "frontend/parser/helpers/parser_test_base.h"

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
                    {E::InvalidExpression, 1, 4, 1, 5}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNull(), IsNumber("2")
                )
            });

            reg({
                .name = "TensorTrailingCommaGarbage",
                .code = "[1, 2, , ]",
                .errors = {
                    {E::InvalidExpression, 1, 8, 1, 9}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNumber("2"), IsNull()
                )
            });

            reg({
                .name = "TensorEmptyElementsStress",
                .code = "[ , , 1, , ]",
                .errors = {
                    {E::InvalidExpression, 1, 3, 1, 4},
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 10, 1, 11}
                },
                .verifier = IsTensor(
                    IsNull(), IsNull(), IsNumber("1"), IsNull()
                )
            });

            reg({
                .name = "EmptyGarbageElement",
                .code = "[*]",
                .errors = {
                    {E::InvalidExpression, 1, 2, 1, 3}
                },
                .verifier = IsTensor(
                    IsNull()
                )
            });

            reg({
                .name = "GarbageElement",
                .code = "[1, *, 3]",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                .verifier = IsTensor(
                    IsNumber("1"), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "InvalidExpressionAsFirst",
                .code = "[*, 1]",
                .errors = {
                    {E::InvalidExpression, 1, 2, 1, 3}
                },
                .verifier = IsTensor(
                    IsNull(), IsNumber("1")
                )
            });

            reg({
                .name = "InvalidExpressionAsSecond",
                .code = "[1, *]",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
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
                    {E::InvalidExpression, 3, 1, 3, 2}
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
                    {E::InvalidExpression, 1, 12, 1, 13}
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
                    {E::UnmatchedBracketAfterTensorElements, 1, 2, 1, 3}
                },
                .verifier = IsTensor(
                    IsNumber("1")
                )
            });

            return true;
        }();
    }
}
