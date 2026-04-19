#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AccessExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(AccessExpressionSuccessPathTest, SimpleDotAccess)
    {
        ExpectValidExpression("obj.property", IsDot(IsIdentifier("obj"), "property"));
    }

    TEST_F(AccessExpressionSuccessPathTest, SimpleDotAccessWithSpace)
    {
        ExpectValidExpression("obj      .         property", IsDot(IsIdentifier("obj"), "property"));
    }

    TEST_F(AccessExpressionSuccessPathTest, ChainedDotAccess)
    {
        ExpectValidExpression("a.b.c",
                              IsDot(
                                  IsDot(IsIdentifier("a"), "b"),
                                  "c"
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, SimpleBracketIndex)
    {
        ExpectValidExpression("arr[0]", IsBracket(IsIdentifier("arr"), IsNumber("0")));
    }

    TEST_F(AccessExpressionSuccessPathTest, SimpleBracketIndexWithSpace)
    {
        ExpectValidExpression("arr      [0]", IsBracket(IsIdentifier("arr"), IsNumber("0")));
    }

    TEST_F(AccessExpressionSuccessPathTest, ChainedBracketAccess)
    {
        ExpectValidExpression("matrix[0][1]",
                              IsBracket(
                                  IsBracket(IsIdentifier("matrix"), IsNumber("0")),
                                  IsNumber("1")
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, NestedBracketAccess)
    {
        ExpectValidExpression("arr[ids[0]]",
                              IsBracket(
                                  IsIdentifier("arr"),
                                  IsBracket(IsIdentifier("ids"), IsNumber("0"))
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, FullSlice)
    {
        ExpectValidExpression("a[0:10]",
                              IsBracket(IsIdentifier("a"),
                                        IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10"))
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, SliceImplicitStart)
    {
        ExpectValidExpression("a[:10]",
                              IsBracket(IsIdentifier("a"),
                                        IsBinary(TokenType::Colon, IsNull(), IsNumber("10"))
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, SliceImplicitEnd)
    {
        ExpectValidExpression("a[0:]",
                              IsBracket(IsIdentifier("a"),
                                        IsBinary(TokenType::Colon, IsNumber("0"), IsNull())
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, SliceFullImplicit)
    {
        ExpectValidExpression("a[:]",
                              IsBracket(IsIdentifier("a"),
                                        IsBinary(TokenType::Colon, IsNull(), IsNull())
                              )
        );
    }

    TEST_F(AccessExpressionSuccessPathTest, SliceWithComplexBounds)
    {
        ExpectValidExpression("a[s + 1 : e - 1]",
                              IsBracket(IsIdentifier("a"),
                                        IsBinary(TokenType::Colon,
                                                 IsBinary(TokenType::Plus, IsIdentifier("s"), IsNumber("1")),
                                                 IsBinary(TokenType::Minus, IsIdentifier("e"), IsNumber("1"))
                                        )
                              )
        );
    }
}
