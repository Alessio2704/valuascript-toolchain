#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "SimpleDotAccess",
                .code = "obj.property",
                .verifier = IsDot(IsIdentifier("obj"), "property")
            });

            reg({
                .name = "DotAccessNewLine",
                .code = "obj\n.property\n.property",
                .verifier = IsDot(IsDot(IsIdentifier("obj"), "property"), "property")
            });

            reg({
                .name = "SimpleDotAccessWithSpace",
                .code = "obj      .         property",
                .verifier = IsDot(IsIdentifier("obj"), "property")
            });

            reg({
                .name = "ChainedDotAccess",
                .code = "a.b.c",
                .verifier = IsDot(
                    IsDot(IsIdentifier("a"), "b"),
                    "c"
                )
            });

            reg({
                .name = "SimpleBracketIndex",
                .code = "arr[0]",
                .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0"))
            });

            reg({
                .name = "SimpleBracketIndexWithSpace",
                .code = "arr      [0]",
                .verifier = IsBracket(IsIdentifier("arr"), IsNumber("0"))
            });

            reg({
                .name = "ChainedBracketAccess",
                .code = "matrix[0][1]",
                .verifier = IsBracket(
                    IsBracket(IsIdentifier("matrix"), IsNumber("0")),
                    IsNumber("1")
                )
            });

            reg({
                .name = "NestedBracketAccess",
                .code = "arr[ids[0]]",
                .verifier = IsBracket(
                    IsIdentifier("arr"),
                    IsBracket(IsIdentifier("ids"), IsNumber("0"))
                )
            });

            reg({
                .name = "FullSlice",
                .code = "a[0:10]",
                .verifier = IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10"))
                )
            });

            reg({
                .name = "SliceImplicitStart",
                .code = "a[:10]",
                .verifier = IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNull(), IsNumber("10"))
                )
            });

            reg({
                .name = "SliceImplicitEnd",
                .code = "a[0:]",
                .verifier = IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNumber("0"), IsNull())
                )
            });

            reg({
                .name = "SliceFullImplicit",
                .code = "a[:]",
                .verifier = IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNull(), IsNull())
                )
            });

            reg({
                .name = "DotAccessSucceedsAtNewlineFuncCallNoArgs",
                .code = "obj.\ntest()",
                .verifier = IsCall(IsDot(IsIdentifier("obj"), "test"), {})
            });

            reg({
                .name = "DotAccessSucceedsAtNewlineFuncCallWithArgs",
                .code = "obj.\ntest(arg: 1)",
                .verifier = IsCall(IsDot(IsIdentifier("obj"), "test"), {
                    {.label = "arg", .value_v = IsNumber("1")}
                })
            });

            return true;
        }();
    }
}
