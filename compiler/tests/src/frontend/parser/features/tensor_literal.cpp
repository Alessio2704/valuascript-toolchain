#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TensorLiteralExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(TensorLiteralExpressionSuccessPathTest, SimpleFlatTensor)
    {
        ExpectValidExpression("[1, 2, 3]", IsTensor({
                                  IsNumber("1"),
                                  IsNumber("2"),
                                  IsNumber("3")
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, EmptyTensor)
    {
        ExpectValidExpression("[]", IsTensor({}));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, MixedTypes)
    {
        ExpectValidExpression("[1, \"a\", true, 5%]", IsTensor({
                                  IsNumber("1"),
                                  IsString("\"a\""),
                                  IsBoolean(true),
                                  IsPercentage("5%")
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, SimpleNestedTensor)
    {
        ExpectValidExpression("[[1], [2]]", IsTensor({
                                  IsTensor({IsNumber("1")}),
                                  IsTensor({IsNumber("2")})
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, ComplexNestedTensor)
    {
        ExpectValidExpression("[[[1, 2, 3], [4, 5, 6], [], []]]", IsTensor({
                                  IsTensor({
                                      IsTensor({IsNumber("1"), IsNumber("2"), IsNumber("3")}),
                                      IsTensor({IsNumber("4"), IsNumber("5"), IsNumber("6")}),
                                      IsTensor({}),
                                      IsTensor({})
                                  })
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, DeepNesting)
    {
        ExpectValidExpression("[[[[1]]]]", IsTensor({
                                  IsTensor({
                                      IsTensor({
                                          IsTensor({IsNumber("1")})
                                      })
                                  })
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, TrailingComma)
    {
        ExpectValidExpression("[1, 2,]", IsTensor({
                                  IsNumber("1"),
                                  IsNumber("2")
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, ExpressionsAsElements)
    {
        ExpectValidExpression("[1 + 2, f(x: 1)]", IsTensor({
                                  IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")),
                                  IsCall(IsIdentifier("f"), {{"x", IsNumber("1")}})
                              }));
    }

    TEST_F(TensorLiteralExpressionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidExpression(
            "[\n"
            "  1,\n"
            "  2,\n"
            "  3\n"
            "]",
            IsTensor({
                IsNumber("1"),
                IsNumber("2"),
                IsNumber("3")
            })
        );
    }
}
