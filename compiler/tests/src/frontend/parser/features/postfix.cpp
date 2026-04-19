#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class PostfixChainingSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(PostfixChainingSuccessPathTest, DotThenCall)
    {
        ExpectValidExpression("obj.method(a: 1)",
                              IsCall(IsDot(IsIdentifier("obj"), "method"), {{"a", IsNumber("1")}}));
    }

    TEST_F(PostfixChainingSuccessPathTest, CallThenDot)
    {
        ExpectValidExpression("get_obj().prop",
                              IsDot(IsCall(IsIdentifier("get_obj"), {}), "prop"));
    }

    TEST_F(PostfixChainingSuccessPathTest, BracketThenCall)
    {
        ExpectValidExpression("arr[0](x: 1)",
                              IsCall(IsBracket(IsIdentifier("arr"), IsNumber("0")), {{"x", IsNumber("1")}}));
    }

    TEST_F(PostfixChainingSuccessPathTest, CallThenBracket)
    {
        ExpectValidExpression("get_arr()[0]",
                              IsBracket(IsCall(IsIdentifier("get_arr"), {}), IsNumber("0")));
    }

    TEST_F(PostfixChainingSuccessPathTest, DotThenBracket)
    {
        ExpectValidExpression("obj.list[0]",
                              IsBracket(IsDot(IsIdentifier("obj"), "list"), IsNumber("0")));
    }

    TEST_F(PostfixChainingSuccessPathTest, BracketThenDot)
    {
        ExpectValidExpression("arr[0].name",
                              IsDot(IsBracket(IsIdentifier("arr"), IsNumber("0")), "name"));
    }

    TEST_F(PostfixChainingSuccessPathTest, MixedChain)
    {
        ExpectValidExpression("a.b[0]().c",
                              IsDot(
                                  IsCall(
                                      IsBracket(
                                          IsDot(IsIdentifier("a"), "b"),
                                          IsNumber("0")
                                      ),
                                      {}
                                  ),
                                  "c"
                              )
        );
    }
}
