#include "expansion_debug_helper.h"

namespace valuascript::compiler::test
{
    class ExpansionRecoveryDebugger : public ExpansionDebugHelper
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ExpansionRecoveryDebugger, GenerateRecoveryReport)
    {
        DebugRecovery("MissingDotAccessProperty");
        DebugRecovery("MissingValueAfterEquals");
        DebugRecovery("BinaryMissingRight");
        DebugRecovery("TupleTypeBrokenElement");
        DebugRecovery("ModifierMissingArgColon");
        DebugRecovery("GenericMissingClosingBracket");
        DebugRecovery("TupleMissingClosingParen");
        DebugRecovery("MultilineBinary");
        DebugRecovery("MultilineBinaryRejectedAfterGroupingCloses");
        DebugRecovery("DanglingBinaryOperatorAtNewline");
        DebugRecovery("UnclosedTensorLiteral");
    }
#endif
}
