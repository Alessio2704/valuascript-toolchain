#include "frontend/parser/helpers/parser_test_base.h"
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
                    DictItemSpec{"key", {}, IsNumber("1")}
                )
            });

            reg({
                .name = "MultipleItems",
                .code = "{ a: 1, b: \"val\", c: true }",
                .verifier = IsDict(
                    DictItemSpec{"a", {}, IsNumber("1")},
                    DictItemSpec{"b", {}, IsString("\"val\"")},
                    DictItemSpec{"c", {}, IsBoolean(true)}
                )
            });

            reg({
                .name = "NestedDictionaries",
                .code = "{ outer: { inner: 1 } }",
                .verifier = IsDict(
                    DictItemSpec{
                        "outer", {}, IsDict(
                            DictItemSpec{"inner", {}, IsNumber("1")}
                        )
                    }
                )
            });

            reg({
                .name = "DictTrailingComma",
                .code = "{ a: 1, b: 2, }",
                .verifier = IsDict(
                    DictItemSpec{"a", {}, IsNumber("1")},
                    DictItemSpec{"b", {}, IsNumber("2")}
                )
            });

            reg({
                .name = "MixedModifiedAndUnmodifiedKeys",
                .code = "{ @sealed a: 1, b: 2, @hidden c: 3 }",
                .verifier = IsDict(
                    DictItemSpec{"a", {{"sealed"}}, IsNumber("1")},
                    DictItemSpec{"b", {}, IsNumber("2")},
                    DictItemSpec{"c", {{"hidden"}}, IsNumber("3")}
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
                    DictItemSpec{"first_name", {}, IsString("\"John\"")},
                    DictItemSpec{"last_name", {}, IsString("\"Doe\"")},
                    DictItemSpec{"age", {}, IsNumber("30")}
                )
            });

            reg({
                .name = "DictComplexRegression",
                .code = "{ x: { y: { z: 1 } } }",
                .verifier = IsDict(
                    DictItemSpec{
                        "x", {}, IsDict(
                            DictItemSpec{
                                "y", {}, IsDict(
                                    DictItemSpec{"z", {}, IsNumber("1")}
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
