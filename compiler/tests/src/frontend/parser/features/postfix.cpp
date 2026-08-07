#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "DotThenCall",
                .code = "obj.method(a: 1)",
                .verifier = IsCall(
                    IsDot(IsIdentifier("obj"), "method"),
                    {{.label="a", .value_v=IsNumber("1")}}
                )
            });

            reg({
                .name = "CallThenDot",
                .code = "get_obj().prop",
                .verifier = IsDot(
                    IsCall(IsIdentifier("get_obj"), {}),
                    "prop"
                )
            });

            reg({
                .name = "BracketThenCall",
                .code = "arr[0](x: 1)",
                .verifier = IsCall(
                    IsBracket(IsIdentifier("arr"), IsNumber("0")),
                    {{.label="x", .value_v=IsNumber("1")}}
                )
            });

            reg({
                .name = "CallThenBracket",
                .code = "get_arr()[0]",
                .verifier = IsBracket(
                    IsCall(IsIdentifier("get_arr"), {}),
                    IsNumber("0")
                )
            });

            reg({
                .name = "DotThenBracket",
                .code = "obj.list[0]",
                .verifier = IsBracket(
                    IsDot(IsIdentifier("obj"), "list"),
                    IsNumber("0")
                )
            });

            reg({
                .name = "BracketThenDot",
                .code = "arr[0].name",
                .verifier = IsDot(
                    IsBracket(IsIdentifier("arr"), IsNumber("0")),
                    "name"
                )
            });

            reg({
                .name = "MixedChain",
                .code = "a.b[0]().c",
                .verifier = IsDot(
                    IsCall(
                        IsBracket(
                            IsDot(IsIdentifier("a"), "b"),
                            IsNumber("0")
                        ),
                        {}
                    ),
                    "c"
                )
            });

            return true;
        }();
    }
}
