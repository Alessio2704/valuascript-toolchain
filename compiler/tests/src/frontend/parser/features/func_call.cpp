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
                .name = "NoArguments",
                .code = "f()",
                .verifier = IsCall(IsIdentifier("f"), {})
            });

            reg({
                .name = "SingleLabeledArgument",
                .code = "f(x: 1)",
                .verifier = IsCall(IsIdentifier("f"), {
                    {"x", IsNumber("1")}
                })
            });

            reg({
                .name = "MultipleLabeledArguments",
                .code = "f(x: 1, y: true, z: \"hi\")",
                .verifier = IsCall(IsIdentifier("f"), {
                    {"x", IsNumber("1")},
                    {"y", IsBoolean(true)},
                    {"z", IsString("\"hi\"")}
                })
            });

            reg({
                .name = "NestedFunctionCallsAsArguments",
                .code = "f(x: g(y: 1))",
                .verifier = IsCall(IsIdentifier("f"), {
                    {
                        "x", IsCall(IsIdentifier("g"), {
                            {"y", IsNumber("1")}
                        })
                    }
                })
            });

            reg({
                .name = "CallReturningFunctionCalledImmediately",
                .code = "get_handler()()",
                .verifier = IsCall(IsCall(IsIdentifier("get_handler"), {}), {})
            });

            reg({
                .name = "CallWithGroupingTarget",
                .code = "(f)(x: 1)",
                .verifier = IsCall(IsGrouping(IsIdentifier("f")), {
                    {"x", IsNumber("1")}
                })
            });

            return true;
        }();
    }
}
