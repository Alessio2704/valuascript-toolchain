#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("EmptyDictionary",
                "{}",
                IsDict({}));

            reg("SingleItem",
                "{ key: 1 }",
                IsDict({
                    {"key", {}, IsNumber("1")}
                }));

            reg("MultipleItems",
                "{ a: 1, b: \"val\", c: true }",
                IsDict({
                    {"a", {}, IsNumber("1")},
                    {"b", {}, IsString("\"val\"")},
                    {"c", {}, IsBoolean(true)}
                }));

            reg("NestedDictionaries",
                "{ outer: { inner: 1 } }",
                IsDict({
                    {
                        "outer", {}, IsDict({
                            {"inner", {}, IsNumber("1")}
                        })
                    }
                }));

            reg("DictTrailingComma",
                "{ a: 1, b: 2, }",
                IsDict({
                    {"a", {}, IsNumber("1")},
                    {"b", {}, IsNumber("2")}
                }));

            reg("MixedModifiedAndUnmodifiedKeys",
                "{ @sealed a: 1, b: 2, @hidden c: 3 }",
                IsDict({
                    {"a", {{"sealed"}}, IsNumber("1")},
                    {"b", {}, IsNumber("2")},
                    {"c", {{"hidden"}}, IsNumber("3")}
                }));

            reg("DictLiteralMultilineFormatting",
                "{\n"
                "  first_name: \"John\",\n"
                "  last_name: \"Doe\",\n"
                "  age: 30\n"
                "}",
                IsDict({
                    {"first_name", {}, IsString("\"John\"")},
                    {"last_name", {}, IsString("\"Doe\"")},
                    {"age", {}, IsNumber("30")}
                }));

            return true;
        }();
    }
}
