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

            return true;
        }();
    }
}
