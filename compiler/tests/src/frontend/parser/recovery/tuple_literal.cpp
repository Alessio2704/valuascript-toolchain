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

            reg("SingleElementTupleNotAllowed", "(1,)",
                {
                    {E::SingleElementTuplesNotAllowed, 1, 3, 1, 4},
                },
                IsTuple(
                    IsNumber("1")
                )
            );

            reg("TrailingCommaInTuple", "(1, 2,)",
                {
                    {E::TrailingCommaInTuple, 1, 6, 1, 7},
                },
                IsTuple(
                    IsNumber("1"),
                    IsNumber("2")
                )
            );

            reg("TupleMissingExpression", "(1, , , 3)",
                {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 7, 1, 8},
                },
                IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            );

            reg("TupleGarbageBetweenElements", "(1, *, *, 3)",
                {
                    {E::InvalidExpression, 1, 5, 1, 6},
                    {E::InvalidExpression, 1, 8, 1, 9},
                },
                IsTuple(
                    IsNumber("1"), IsNull(), IsNull(), IsNumber("3")
                )
            );

            reg("EmptyTupleWithGarbage", "(*)",
                {
                    {E::InvalidExpression, 1, 2, 1, 3}
                },
                IsGrouping(IsNull())
            );

            reg("TupleWithColonInsteadOfComma", "(1, x: 2)",
                {
                    {E::ExpectedRightParenAfterTupleElements, 1, 6, 1, 7}
                },
                IsTuple(
                    IsNumber("1"),
                    IsIdentifier("x")
                )
            );

            reg("TupleMultilineRecovery",
                "(\n"
                "  1,\n"
                "  *,\n"
                "  3\n"
                ")",
                {
                    {E::InvalidExpression, 3, 3, 3, 4}
                },
                IsTuple(
                    IsNumber("1"),
                    IsNull(),
                    IsNumber("3")
                )
            );

            reg("EmptyTupleWithCommaIsInvalid", "(,)",
                {
                    {E::InvalidExpression, 1, 2, 1, 3},
                    {E::SingleElementTuplesNotAllowed, 1, 2, 1, 3}
                },
                IsTuple()
            );

            return true;
        }();
    }
}
