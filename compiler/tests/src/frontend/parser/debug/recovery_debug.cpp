#include "expansion_debug_helper.h"
#include "utils/parametrised_test_name_helper.h"

namespace valuascript::compiler::test
{
    class ExpansionRecoveryDebugger : public ExpansionDebugHelper,
                                      public testing::WithParamInterface<DumpTest>
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_P(ExpansionRecoveryDebugger, GenerateRecoveryDump)
    {
        DebugRecovery(GetParam().name);
    }

    INSTANTIATE_TEST_SUITE_P(
        RecoveryDumps,
        ExpansionRecoveryDebugger,
        testing::Values(
            DumpTest{.name = "MissingDotAccessProperty"},
            DumpTest{.name = "MissingValueAfterEquals"},
            DumpTest{.name = "BinaryMissingRight"},
            DumpTest{.name = "TupleTypeBrokenElement"},
            DumpTest{.name = "ModifierMissingArgColon"},
            DumpTest{.name = "GenericMissingClosingBracket"},
            DumpTest{.name = "TupleMissingClosingParen"},
            DumpTest{.name = "MultilineBinary"},
            DumpTest{.name = "MultilineBinaryRejectedAfterGroupingCloses"},
            DumpTest{.name = "DanglingBinaryOperatorAtNewline"},
            DumpTest{.name = "UnclosedTensorLiteral"},
            DumpTest{.name = "EnumMissingRightBrace"},
            DumpTest{.name = "MissingRightBraceInFunctionBody"},
            DumpTest{.name = "BracketAccessMissingClosingBracket"},
            DumpTest{.name = "BracketSliceMissingClosingBracket"},
            DumpTest{.name = "SwitchMissingClosingBrace"},
            DumpTest{.name = "GroupingMissingClosingParen"},
            DumpTest{.name = "ModifierMissingClosingParen"},
            DumpTest{.name = "DictMissingClosingBrace"}
        ),
        TestNameGenerator{}
    );
#endif
}

