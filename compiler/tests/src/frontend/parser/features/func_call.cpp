#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("NoArguments",
                "f()",
                IsCall(IsIdentifier("f"), {}));

            reg("SingleLabeledArgument",
                "f(x: 1)",
                IsCall(IsIdentifier("f"), {
                           {"x", IsNumber("1")}
                       }));

            reg("MultipleLabeledArguments",
                "f(x: 1, y: true, z: \"hi\")",
                IsCall(IsIdentifier("f"), {
                           {"x", IsNumber("1")},
                           {"y", IsBoolean(true)},
                           {"z", IsString("\"hi\"")}
                       }));

            reg("NestedFunctionCallsAsArguments",
                "f(x: g(y: 1))",
                IsCall(IsIdentifier("f"), {
                           {
                               "x", IsCall(IsIdentifier("g"), {
                                               {"y", IsNumber("1")}
                                           })
                           }
                       }));

            reg("CallReturningFunctionCalledImmediately",
                "get_handler()()",
                IsCall(IsCall(IsIdentifier("get_handler"), {}), {}));

            reg("CallWithGroupingTarget",
                "(f)(x: 1)",
                IsCall(IsGrouping(IsIdentifier("f")), {
                           {"x", IsNumber("1")}
                       }));

            return true;
        }();
    }
}
