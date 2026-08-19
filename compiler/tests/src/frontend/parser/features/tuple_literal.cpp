#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "EmptyTuple",
                .code = "()",
                .verifier = IsTuple()
            });

            reg({
                .name = "StandardPair",
                .code = "(1, 2)",
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNumber("2")
                )
            });

            reg({
                .name = "StandardTriple",
                .code = "(1, 2, 3)",
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNumber("2"),
                    IsNumber("3")
                )
            });

            reg({
                .name = "TupleMixedTypes",
                .code = "(1, \"a\", true)",
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsString("\"a\""),
                    IsBoolean(true)
                )
            });

            reg({
                .name = "SimpleNestedTuples",
                .code = "((1, 2), 3)",
                .verifier = IsTuple(
                    IsTuple(IsNumber("1"), IsNumber("2")),
                    IsNumber("3")
                )
            });

            reg({
                .name = "TupleDifferentiateFromGrouping",
                .code = "((1 + 2), 3)",
                .verifier = IsTuple(
                    IsGrouping(
                        IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))),
                    IsNumber("3")
                )
            });

            reg({
                .name = "ComplexNestedTuples",
                .code = "((1, 2), (3, (4, 5)))",
                .verifier = IsTuple(
                    IsTuple(IsNumber("1"), IsNumber("2")),
                    IsTuple(
                        IsNumber("3"),
                        IsTuple(IsNumber("4"), IsNumber("5"))
                    )
                )
            });

            reg({
                .name = "MultilineFormatting",
                .code = "(\n"
                "  1,\n"
                "  2\n"
                ")",
                .verifier = IsTuple(
                    IsNumber("1"),
                    IsNumber("2")
                )
            });

            reg({
                .name = "DistinctionFromGrouping",
                .code = "(1)",
                .verifier = IsGrouping(IsNumber("1"))
            });

            reg({
                .name = "TupleComplexRegression",
                .code = "(( ( 1 ) ))",
                .verifier = IsGrouping(
                    IsGrouping(
                        IsGrouping(
                            IsNumber("1")
                        )
                    )
                )
            });

            return true;
        }();
    }
}
