#include "expansion_debug_helper.h"

namespace valuascript::compiler::test
{
    class ExpansionDebugger : public ExpansionDebugHelper
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ExpansionDebugger, GenerateExpressionReport)
    {
        DumpFeatureExpansion(InjectableType::Expression, "1 + 1", "BasicArithmetic");
        DumpFeatureExpansion(InjectableType::TypeAnnotation, "int", "BasicType");
        DumpFeatureExpansion(InjectableType::Modifier, "@modifier", "BasicModifier");
        DumpFeatureExpansion(InjectableType::WeakStatement, "return 1", "BasicReturn");
        DumpFeatureExpansion(InjectableType::StrongStatement, "let res = 1", "BasicAssign");
    }
#endif
}
