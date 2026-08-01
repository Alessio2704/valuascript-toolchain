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
                .name = "SingleElementTupleNotAllowed",
                .code = "(1,)",
                .errors = {
                    {E::SingleElementTuplesNotAllowed, 1, 3, 1, 4}
                },
                .verifier = IsTuple(
                    IsNumber("1")
                )
            });

            reg({
                .name = "TrailingCommaInTuple",
                .code = "(1, 2,)",
                .errors = {
                    {E::TrailingCommaInTuple, 1, 6, 1, 7}
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNumber("2")
                )
            });

            reg({
                .name = "TupleMissingExpression",
                .code = "(1, , , 3)",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 7, 1, 8}
                },
                .verifier = IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "TupleGarbageBetweenElements",
                .code = "(1, *, *, 3)",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 8, 1, 9}
                },
                .verifier = IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "EmptyTupleWithGarbage",
                .code = "(*)",
                .errors = {
                    {E::InvalidExpression, 1, 2, 1, 3}
                },
                .verifier = IsGrouping(IsNull())
            });

            reg({
                .name = "TupleWithColonInsteadOfComma",
                .code = "(1, x: 2)",
                .errors = {
                    {E::ExpectedRightParenAfterTupleElements, 1, 5, 1, 6}
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsIdentifier("x")
                )
            });

            reg({
                .name = "TupleMultilineRecovery",
                .code = "(\n"
                "  1,\n"
                "  *,\n"
                "  3\n"
                ")",
                .errors = {
                    {E::InvalidExpression, 3, 3, 3, 4}
                },
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNull(),
                    IsNumber("3")
                )
            });

            reg({
                .name = "EmptyTupleWithCommaIsInvalid",
                .code = "(,)",
                .errors = {
                    {E::InvalidExpression, 1, 2, 1, 3},
                    {E::SingleElementTuplesNotAllowed, 1, 2, 1, 3}
                },
                .verifier = IsTuple()
            });

            return true;
        }();
    }
}
