#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TupleLiteralExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(TupleLiteralExpressionSuccessPathTest, EmptyTuple)
    {
        ExpectValidExpression("()", IsTuple({}));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, StandardPair)
    {
        ExpectValidExpression("(1, 2)", IsTuple({
                                  IsNumber("1"),
                                  IsNumber("2")
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, StandardTriple)
    {
        ExpectValidExpression("(1, 2, 3)", IsTuple({
                                  IsNumber("1"),
                                  IsNumber("2"),
                                  IsNumber("3")
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, MixedTypes)
    {
        ExpectValidExpression("(1, \"a\", true)", IsTuple({
                                  IsNumber("1"),
                                  IsString("\"a\""),
                                  IsBoolean(true)
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, SimpleNestedTuples)
    {
        ExpectValidExpression("((1, 2), 3)", IsTuple({
                                  IsTuple({IsNumber("1"), IsNumber("2")}),
                                  IsNumber("3")
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, TupleDifferenciateFromGrouping)
    {
        ExpectValidExpression("((1 + 2), 3)", IsTuple({
                                  IsGrouping(
                                      IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))),
                                  IsNumber("3")
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, ComplexNestedTuples)
    {
        ExpectValidExpression("((1, 2), (3, (4, 5)))", IsTuple({
                                  IsTuple({IsNumber("1"), IsNumber("2")}),
                                  IsTuple({
                                      IsNumber("3"),
                                      IsTuple({IsNumber("4"), IsNumber("5")})
                                  })
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, ExpressionsAsElements)
    {
        ExpectValidExpression("(1 + 2, f(x: 1))", IsTuple({
                                  IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")),
                                  IsCall(IsIdentifier("f"), {{"x", IsNumber("1")}})
                              }));
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidExpression(
            "(\n"
            "  1,\n"
            "  2\n"
            ")",
            IsTuple({
                IsNumber("1"),
                IsNumber("2")
            })
        );
    }

    TEST_F(TupleLiteralExpressionSuccessPathTest, DistinctionFromGrouping)
    {
        ExpectValidExpression("(1)", IsGrouping(IsNumber("1")));
    }
}
