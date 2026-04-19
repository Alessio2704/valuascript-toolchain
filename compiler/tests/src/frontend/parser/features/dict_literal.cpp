#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DictLiteralExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(DictLiteralExpressionSuccessPathTest, EmptyDictionary)
    {
        ExpectValidExpression("{}", IsDict({}));
    }

    TEST_F(DictLiteralExpressionSuccessPathTest, SingleItem)
    {
        ExpectValidExpression("{ key: 1 }", IsDict({
                                  {"key", {}, IsNumber("1")}
                              }));
    }

    TEST_F(DictLiteralExpressionSuccessPathTest, MultipleItems)
    {
        ExpectValidExpression("{ a: 1, b: \"val\", c: true }", IsDict({
                                  {"a", {}, IsNumber("1")},
                                  {"b", {}, IsString("\"val\"")},
                                  {"c", {}, IsBoolean(true)}
                              }));
    }

    TEST_F(DictLiteralExpressionSuccessPathTest, NestedDictionaries)
    {
        ExpectValidExpression("{ outer: { inner: 1 } }", IsDict({
                                  {
                                      "outer", {}, IsDict({
                                          {"inner", {}, IsNumber("1")}
                                      })
                                  }
                              }));
    }

    TEST_F(DictLiteralExpressionSuccessPathTest, TrailingComma)
    {
        ExpectValidExpression("{ a: 1, b: 2, }", IsDict({
                                  {"a", {}, IsNumber("1")},
                                  {"b", {}, IsNumber("2")}
                              }));
    }

    TEST_F(DictLiteralExpressionSuccessPathTest, MixedModifiedAndUnmodifiedKeys)
    {
        ExpectValidExpression("{ @sealed a: 1, b: 2, @hidden c: 3 }", IsDict({
                                  {"a", {{"sealed"}}, IsNumber("1")},
                                  {"b", {}, IsNumber("2")},
                                  {"c", {{"hidden"}}, IsNumber("3")}
                              }));
    }

    TEST_F(DictLiteralExpressionSuccessPathTest, MultilineFormatting)
    {
        ExpectValidExpression(
            "{\n"
            "  first_name: \"John\",\n"
            "  last_name: \"Doe\",\n"
            "  age: 30\n"
            "}",
            IsDict({
                {"first_name", {}, IsString("\"John\"")},
                {"last_name", {}, IsString("\"Doe\"")},
                {"age", {}, IsNumber("30")}
            })
        );
    }
}
