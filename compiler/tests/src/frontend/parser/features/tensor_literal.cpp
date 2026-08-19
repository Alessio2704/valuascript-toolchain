#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "SimpleFlatTensor",
                .code = "[1, 2, 3]",
                .verifier = IsTensor(
                    IsNumber("1"),
                    IsNumber("2"),
                    IsNumber("3")
                )
            });

            reg({
                .name = "EmptyTensor",
                .code = "[]",
                .verifier = IsTensor()
            });

            reg({
                .name = "TensorMixedTypes",
                .code = "[1, \"a\", true, 5%]",
                .verifier = IsTensor(
                    IsNumber("1"),
                    IsString("\"a\""),
                    IsBoolean(true),
                    IsPercentage("5%")
                )
            });

            reg({
                .name = "SimpleNestedTensor",
                .code = "[[1], [2]]",
                .verifier = IsTensor(
                    IsTensor(IsNumber("1")),
                    IsTensor(IsNumber("2"))
                )
            });

            reg({
                .name = "ComplexNestedTensor",
                .code = "[[[1, 2, 3], [4, 5, 6], [], []]]",
                .verifier = IsTensor(
                    IsTensor(
                        IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        IsTensor(IsNumber("4"), IsNumber("5"), IsNumber("6")),
                        IsTensor(),
                        IsTensor()
                    )
                )
            });

            reg({
                .name = "DeepNesting",
                .code = "[[[[1]]]]",
                .verifier = IsTensor(
                    IsTensor(
                        IsTensor(
                            IsTensor(IsNumber("1"))
                        )
                    )
                )
            });

            reg({
                .name = "TensorTrailingComma",
                .code = "[1, 2,]",
                .verifier = IsTensor(
                    IsNumber("1"),
                    IsNumber("2")
                )
            });

            reg({
                .name = "TensorLiteralMultilineFormatting",
                .code = "[\n"
                "  1,\n"
                "  2,\n"
                "  3\n"
                "]",
                .verifier = IsTensor(
                    IsNumber("1"),
                    IsNumber("2"),
                    IsNumber("3")
                )
            });

            return true;
        }();
    }
}
