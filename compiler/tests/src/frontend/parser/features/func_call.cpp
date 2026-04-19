#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class FunctionCallExpressionSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(FunctionCallExpressionSuccessPathTest, NoArguments)
    {
        ExpectValidExpression("f()", IsCall(IsIdentifier("f"), {}));
    }

    TEST_F(FunctionCallExpressionSuccessPathTest, SingleLabeledArgument)
    {
        ExpectValidExpression("f(x: 1)", IsCall(IsIdentifier("f"), {
                                                    {"x", IsNumber("1")}
                                                }));
    }

    TEST_F(FunctionCallExpressionSuccessPathTest, MultipleLabeledArguments)
    {
        ExpectValidExpression("f(x: 1, y: true, z: \"hi\")", IsCall(IsIdentifier("f"), {
                                                                        {"x", IsNumber("1")},
                                                                        {"y", IsBoolean(true)},
                                                                        {"z", IsString("\"hi\"")}
                                                                    }));
    }

    TEST_F(FunctionCallExpressionSuccessPathTest, NestedFunctionCallsAsArguments)
    {
        ExpectValidExpression("f(x: g(y: 1))",
                              IsCall(IsIdentifier("f"), {
                                         {
                                             "x", IsCall(IsIdentifier("g"), {
                                                             {"y", IsNumber("1")}
                                                         })
                                         }
                                     })
        );
    }

    TEST_F(FunctionCallExpressionSuccessPathTest, CallReturningFunctionCalledImmediately)
    {
        ExpectValidExpression("get_handler()()",
                              IsCall(IsCall(IsIdentifier("get_handler"), {}), {})
        );
    }


    TEST_F(FunctionCallExpressionSuccessPathTest, CallWithGroupingTarget)
    {
        ExpectValidExpression("(f)(x: 1)",
                              IsCall(IsGrouping(IsIdentifier("f")), {
                                         {"x", IsNumber("1")}
                                     })
        );
    }
}
