#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("SimpleDotAccess",
                "obj.property",
                IsDot(IsIdentifier("obj"), "property"));

            reg("SimpleDotAccessWithSpace",
                "obj      .         property",
                IsDot(IsIdentifier("obj"), "property"));

            reg("ChainedDotAccess",
                "a.b.c",
                IsDot(
                    IsDot(IsIdentifier("a"), "b"),
                    "c"
                ));

            reg("SimpleBracketIndex",
                "arr[0]",
                IsBracket(IsIdentifier("arr"), IsNumber("0")));

            reg("SimpleBracketIndexWithSpace",
                "arr      [0]",
                IsBracket(IsIdentifier("arr"), IsNumber("0")));

            reg("ChainedBracketAccess",
                "matrix[0][1]",
                IsBracket(
                    IsBracket(IsIdentifier("matrix"), IsNumber("0")),
                    IsNumber("1")
                ));

            reg("NestedBracketAccess",
                "arr[ids[0]]",
                IsBracket(
                    IsIdentifier("arr"),
                    IsBracket(IsIdentifier("ids"), IsNumber("0"))
                ));

            reg("FullSlice",
                "a[0:10]",
                IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10"))
                ));

            reg("SliceImplicitStart",
                "a[:10]",
                IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNull(), IsNumber("10"))
                ));

            reg("SliceImplicitEnd",
                "a[0:]",
                IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNumber("0"), IsNull())
                ));

            reg("SliceFullImplicit",
                "a[:]",
                IsBracket(
                    IsIdentifier("a"),
                    IsBinary(TokenType::Colon, IsNull(), IsNull())
                ));

            return true;
        }();
    }
}
