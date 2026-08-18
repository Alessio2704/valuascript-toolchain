#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "EmptyDictionary",
                .code = "{}",
                .verifier = IsDict()
            });

            reg({
                .name = "SingleItem",
                .code = "{ key: 1 }",
                .verifier = IsDict(
                    DictItemSpec{.key = "key", .value_v = IsNumber("1")}
                )
            });

            reg({
                .name = "MultipleItems",
                .code = "{ a: 1, b: \"val\", c: true }",
                .verifier = IsDict(
                    DictItemSpec{.key = "a", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "b", .value_v = IsString("\"val\"")},
                    DictItemSpec{.key = "c", .value_v = IsBoolean(true)}
                )
            });

            reg({
                .name = "NestedDictionaries",
                .code = "{ outer: { inner: 1 } }",
                .verifier = IsDict(
                    DictItemSpec{
                        .key = "outer", .value_v = IsDict(
                            DictItemSpec{.key = "inner", .value_v = IsNumber("1")}
                        )
                    }
                )
            });

            reg({
                .name = "DictTrailingComma",
                .code = "{ a: 1, b: 2, }",
                .verifier = IsDict(
                    DictItemSpec{.key = "a", .value_v = IsNumber("1")},
                    DictItemSpec{.key = "b", .value_v = IsNumber("2")}
                )
            });

            reg({
                .name = "MixedModifiedAndUnmodifiedKeys",
                .code = "{ @sealed a: 1, b: 2, @hidden c: 3 }",
                .verifier = IsDict(
                    DictItemSpec{.key = "a", .modifiers = {{.name="sealed"}}, .value_v = IsNumber("1")},
                    DictItemSpec{.key = "b", .value_v = IsNumber("2")},
                    DictItemSpec{.key = "c", .modifiers = {{.name="hidden"}}, .value_v = IsNumber("3")}
                )
            });

            reg({
                .name = "DictLiteralMultilineFormatting",
                .code = "{\n"
                "  first_name: \"John\",\n"
                "  last_name: \"Doe\",\n"
                "  age: 30\n"
                "}",
                .verifier = IsDict(
                    DictItemSpec{.key = "first_name", .value_v = IsString("\"John\"")},
                    DictItemSpec{.key = "last_name", .value_v = IsString("\"Doe\"")},
                    DictItemSpec{.key = "age", .value_v = IsNumber("30")}
                )
            });

            reg({
                .name = "DictComplexRegression",
                .code = "{ x: { y: { z: 1 } } }",
                .verifier = IsDict(
                    DictItemSpec{
                        .key = "x", .value_v = IsDict(
                            DictItemSpec{
                                .key = "y", .value_v = IsDict(
                                    DictItemSpec{.key = "z", .value_v = IsNumber("1")}
                                )
                            }
                        )
                    }
                )
            });

            return true;
        }();
    }
}
