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
                    PErr{
                        .code = E::SingleElementTuplesNotAllowed, .line_start = 1, .column_start = 3, .line_end = 1,
                        .column_end = 4
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1")
                )
            });

            reg({
                .name = "TrailingCommaInTuple",
                .code = "(1, 2,)",
                .errors = {
                    PErr{
                        .code = E::TrailingCommaInTuple, .line_start = 1, .column_start = 6, .line_end = 1,
                        .column_end = 7
                    }
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
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    },
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "TupleGarbageBetweenElements",
                .code = "(1, *, *, 3)",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    },
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9
                    }
                },
                .verifier = IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            });

            reg({
                .name = "EmptyTupleWithGarbage",
                .code = "(*)",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3
                    }
                },
                .verifier = IsGrouping(IsNull())
            });

            reg({
                .name = "TupleWithColonInsteadOfComma",
                .code = "(1, x: 2)",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 5,
                        .line_end = 1, .column_end = 6
                    }
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
                    PErr{
                        .code = E::InvalidExpression, .line_start = 3, .column_start = 3, .line_end = 3, .column_end = 4
                    }
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
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3
                    },
                    PErr{
                        .code = E::SingleElementTuplesNotAllowed, .line_start = 1, .column_start = 2, .line_end = 1,
                        .column_end = 3
                    }
                },
                .verifier = IsTuple(IsNull())
            });

            return true;
        }();
    }
}
