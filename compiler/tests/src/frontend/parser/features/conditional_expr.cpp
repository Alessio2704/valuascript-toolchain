#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ConditionalExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ConditionalExpressionSuccessPathTest, SimpleConditional)
    {
        ExpectValidExpression("if a then 1 else 0",
                              IsConditional(
                                  IsIdentifier("a"),
                                  IsNumber("1"),
                                  IsNumber("0")
                              )
        );
    }

    TEST_F(ConditionalExpressionSuccessPathTest, NestedElseIf)
    {
        ExpectValidExpression("if a then 1 else if b then 2 else 3",
                              IsConditional(
                                  IsIdentifier("a"),
                                  IsNumber("1"),
                                  IsConditional(
                                      IsIdentifier("b"),
                                      IsNumber("2"),
                                      IsNumber("3")
                                  )
                              )
        );
    }

    TEST_F(ConditionalExpressionSuccessPathTest, NestedThen)
    {
        ExpectValidExpression("if a then if b then 1 else 2 else 3",
                              IsConditional(
                                  IsIdentifier("a"),
                                  IsConditional(
                                      IsIdentifier("b"),
                                      IsNumber("1"),
                                      IsNumber("2")
                                  ),
                                  IsNumber("3")
                              )
        );
    }

    TEST_F(ConditionalExpressionSuccessPathTest, DeeplyNested)
    {
        ExpectValidExpression("if a then 1 else if b then 2 else if c then 3 else 4",
                              IsConditional(
                                  IsIdentifier("a"),
                                  IsNumber("1"),
                                  IsConditional(
                                      IsIdentifier("b"),
                                      IsNumber("2"),
                                      IsConditional(
                                          IsIdentifier("c"),
                                          IsNumber("3"),
                                          IsNumber("4")
                                      )
                                  )
                              )
        );
    }

    TEST_F(ConditionalExpressionSuccessPathTest, ComplexExpressionsInBranches)
    {
        ExpectValidExpression("if x > 10 then f(a: 1) else 100 * 2",
                              IsConditional(
                                  IsBinary(TokenType::Greater, IsIdentifier("x"), IsNumber("10")),
                                  IsCall(IsIdentifier("f"), {{"a", IsNumber("1")}}),
                                  IsBinary(TokenType::Star, IsNumber("100"), IsNumber("2"))
                              )
        );
    }

    TEST_F(ConditionalExpressionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidExpression(
            "if is_valid\n"
            "then \"yes\"\n"
            "else \"no\"",
            IsConditional(
                IsIdentifier("is_valid"),
                IsString("\"yes\""),
                IsString("\"no\"")
            )
        );
    }
}
