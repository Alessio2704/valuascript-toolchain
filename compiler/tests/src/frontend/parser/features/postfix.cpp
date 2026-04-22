#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("DotThenCall",
                "obj.method(a: 1)",
                IsCall(
                    IsDot(IsIdentifier("obj"), "method"),
                    {{"a", IsNumber("1")}}
                ));

            reg("CallThenDot",
                "get_obj().prop",
                IsDot(
                    IsCall(IsIdentifier("get_obj"), {}),
                    "prop"
                ));

            reg("BracketThenCall",
                "arr[0](x: 1)",
                IsCall(
                    IsBracket(IsIdentifier("arr"), IsNumber("0")),
                    {{"x", IsNumber("1")}}
                ));

            reg("CallThenBracket",
                "get_arr()[0]",
                IsBracket(
                    IsCall(IsIdentifier("get_arr"), {}),
                    IsNumber("0")
                ));

            reg("DotThenBracket",
                "obj.list[0]",
                IsBracket(
                    IsDot(IsIdentifier("obj"), "list"),
                    IsNumber("0")
                ));

            reg("BracketThenDot",
                "arr[0].name",
                IsDot(
                    IsBracket(IsIdentifier("arr"), IsNumber("0")),
                    "name"
                ));

            reg("MixedChain",
                "a.b[0]().c",
                IsDot(
                    IsCall(
                        IsBracket(
                            IsDot(IsIdentifier("a"), "b"),
                            IsNumber("0")
                        ),
                        {}
                    ),
                    "c"
                ));

            return true;
        }();
    }
}
