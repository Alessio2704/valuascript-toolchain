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
                .name = "SimpleConditional",
                .code = "if a then 1 else 0",
                .verifier = IsConditional(
                    IsIdentifier("a"),
                    IsNumber("1"),
                    IsNumber("0")
                )
            });

            reg({
                .name = "NestedElseIf",
                .code = "if a then 1 else if b then 2 else 3",
                .verifier = IsConditional(
                    IsIdentifier("a"),
                    IsNumber("1"),
                    IsConditional(
                        IsIdentifier("b"),
                        IsNumber("2"),
                        IsNumber("3")
                    )
                )
            });

            reg({
                .name = "NestedThen",
                .code = "if a then if b then 1 else 2 else 3",
                .verifier = IsConditional(
                    IsIdentifier("a"),
                    IsConditional(
                        IsIdentifier("b"),
                        IsNumber("1"),
                        IsNumber("2")
                    ),
                    IsNumber("3")
                )
            });

            reg({
                .name = "DeeplyNested",
                .code = "if a then 1 else if b then 2 else if c then 3 else 4",
                .verifier = IsConditional(
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
            });

            reg({
                .name = "ConditionalExprMultilineFormatting",
                .code = "if is_valid\n"
                "then \"yes\"\n"
                "else \"no\"",
                .verifier = IsConditional(
                    IsIdentifier("is_valid"),
                    IsString("\"yes\""),
                    IsString("\"no\"")
                )
            });

            return true;
        }();
    }
}
