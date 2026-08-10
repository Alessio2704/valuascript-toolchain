#include "expansion_debug_helper.h"

namespace valuascript::compiler::test
{
    class ExpansionFeatureDebugger : public ExpansionDebugHelper
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ExpansionFeatureDebugger, GenerateExpressionReport)
    {
        DebugFeature("IntegerLiteral");
        DebugFeature("Generic");
        DebugFeature("SingleModifier");
        DebugFeature("ReturnSingleValue");
        DebugFeature("ExplicitType");
    }
#endif
}
