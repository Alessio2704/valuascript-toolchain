#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("SimpleConditional",
                "if a then 1 else 0",
                IsConditional(
                    IsIdentifier("a"),
                    IsNumber("1"),
                    IsNumber("0")
                ));

            reg("NestedElseIf",
                "if a then 1 else if b then 2 else 3",
                IsConditional(
                    IsIdentifier("a"),
                    IsNumber("1"),
                    IsConditional(
                        IsIdentifier("b"),
                        IsNumber("2"),
                        IsNumber("3")
                    )
                ));

            reg("NestedThen",
                "if a then if b then 1 else 2 else 3",
                IsConditional(
                    IsIdentifier("a"),
                    IsConditional(
                        IsIdentifier("b"),
                        IsNumber("1"),
                        IsNumber("2")
                    ),
                    IsNumber("3")
                ));

            reg("DeeplyNested",
                "if a then 1 else if b then 2 else if c then 3 else 4",
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
                ));

            reg("ConditionalExprMultilineFormatting",
                "if is_valid\n"
                "then \"yes\"\n"
                "else \"no\"",
                IsConditional(
                    IsIdentifier("is_valid"),
                    IsString("\"yes\""),
                    IsString("\"no\"")
                ));

            return true;
        }();
    }
}
