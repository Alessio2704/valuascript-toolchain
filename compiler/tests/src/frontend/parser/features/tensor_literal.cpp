#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("SimpleFlatTensor",
                "[1, 2, 3]",
                IsTensor(
                    IsNumber("1"),
                    IsNumber("2"),
                    IsNumber("3")
                ));

            reg("EmptyTensor",
                "[]",
                IsTensor());

            reg("TensorMixedTypes",
                "[1, \"a\", true, 5%]",
                IsTensor(
                    IsNumber("1"),
                    IsString("\"a\""),
                    IsBoolean(true),
                    IsPercentage("5%")
                ));

            reg("SimpleNestedTensor",
                "[[1], [2]]",
                IsTensor(
                    IsTensor(IsNumber("1")),
                    IsTensor(IsNumber("2"))
                ));

            reg("ComplexNestedTensor",
                "[[[1, 2, 3], [4, 5, 6], [], []]]",
                IsTensor(
                    IsTensor(
                        IsTensor(IsNumber("1"), IsNumber("2"), IsNumber("3")),
                        IsTensor(IsNumber("4"), IsNumber("5"), IsNumber("6")),
                        IsTensor(),
                        IsTensor()
                    )
                ));

            reg("DeepNesting",
                "[[[[1]]]]",
                IsTensor(
                    IsTensor(
                        IsTensor(
                            IsTensor(IsNumber("1"))
                        )
                    )
                ));

            reg("TensorTrailingComma",
                "[1, 2,]",
                IsTensor(
                    IsNumber("1"),
                    IsNumber("2")
                ));

            reg("TensorLiteralMultilineFormatting",
                "[\n"
                "  1,\n"
                "  2,\n"
                "  3\n"
                "]",
                IsTensor(
                    IsNumber("1"),
                    IsNumber("2"),
                    IsNumber("3")
                ));

            return true;
        }();
    }
}
