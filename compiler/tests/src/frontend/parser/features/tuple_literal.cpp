#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("EmptyTuple",
                "()",
                IsTuple({}));

            reg("StandardPair",
                "(1, 2)",
                IsTuple({
                    IsNumber("1"),
                    IsNumber("2")
                }));

            reg("StandardTriple",
                "(1, 2, 3)",
                IsTuple({
                    IsNumber("1"),
                    IsNumber("2"),
                    IsNumber("3")
                }));

            reg("TupleMixedTypes",
                "(1, \"a\", true)",
                IsTuple({
                    IsNumber("1"),
                    IsString("\"a\""),
                    IsBoolean(true)
                }));

            reg("SimpleNestedTuples",
                "((1, 2), 3)",
                IsTuple({
                    IsTuple({IsNumber("1"), IsNumber("2")}),
                    IsNumber("3")
                }));

            reg("TupleDifferenciateFromGrouping",
                "((1 + 2), 3)",
                IsTuple({
                    IsGrouping(
                        IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))),
                    IsNumber("3")
                }));

            reg("ComplexNestedTuples",
                "((1, 2), (3, (4, 5)))",
                IsTuple({
                    IsTuple({IsNumber("1"), IsNumber("2")}),
                    IsTuple({
                        IsNumber("3"),
                        IsTuple({IsNumber("4"), IsNumber("5")})
                    })
                }));

            reg("MultilineFormatting",
                "(\n"
                "  1,\n"
                "  2\n"
                ")",
                IsTuple({
                    IsNumber("1"),
                    IsNumber("2")
                }));

            reg("DistinctionFromGrouping",
                "(1)",
                IsGrouping(IsNumber("1")));

            return true;
        }();
    }
}
