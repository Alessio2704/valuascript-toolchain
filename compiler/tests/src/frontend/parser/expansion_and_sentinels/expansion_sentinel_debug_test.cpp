#include "expansion_debug_helper.h"

namespace valuascript::compiler::test
{
    class ExpansionRecoveryDebugger : public ExpansionDebugHelper
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ExpansionRecoveryDebugger, GenerateRecoveryReport)
    {
        DumpRecoveryExpansion(
            InjectableType::Expression,
            "obj.",
            "MissingDotAccessProperty",
            {},
            {
                ContextOverride<>{
                    .context_name = ContextNames::ExprSingleAssignment,
                    .accepted_sentinels = SentinelKinds::all()
                }
            },
            {SentinelKind::ExprStmt}
        );

        DumpRecoveryExpansion(
            InjectableType::StrongStatement,
            "let x = ",
            "BrokenAssignment"
        );

        DumpRecoveryExpansion(
            InjectableType::StrongStatement,
            "1 + 1",
            "InvalidStandaloneStatement"
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1 + ",
            "MalformedBinary"
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "map<string, *, int>",
            "BrokenTypeAnnotation"
        );

        DumpRecoveryExpansion(
            InjectableType::Modifier,
            "@test(a 1, b: 2)",
            "BrokenModifier"
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "vector<int",
            "GenericMissingClosingBracket",
            {
                ContextNames::TypeTupleTypeStart,
                ContextNames::TypeTupleTypeMiddle,
                ContextNames::TypeGenericTypeStart,
                ContextNames::TypeGenericTypeMiddle,
                ContextNames::TypeGenericTypeEnd
            },
            {
                ContextOverride{.context_name = ContextNames::TypeMultiAssignmentTarget1},
                ContextOverride{.context_name = ContextNames::TypeFunctionMultiReturn},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeStart},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeMiddle},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeStart},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeMiddle},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeEnd}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "(int, string",
            "TupleMissingClosingParen",
            {
                ContextNames::TypeTupleTypeStart,
                ContextNames::TypeTupleTypeMiddle,
                ContextNames::TypeTupleTypeEnd,
                ContextNames::TypeGenericTypeStart,
                ContextNames::TypeGenericTypeMiddle,
                ContextNames::TypeGenericTypeEnd
            },
            {
                ContextOverride{.context_name = ContextNames::TypeFunctionParameter},
                ContextOverride{.context_name = ContextNames::TypeFunctionMultiParameter2},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeEnd},
                ContextOverride{.context_name = ContextNames::TypeMultiAssignmentTarget1},
                ContextOverride{.context_name = ContextNames::TypeFunctionMultiReturn},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeStart},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeMiddle},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeStart},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeMiddle},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeEnd}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1\n* 2\n",
            "MultilineBinary",
            {
                ContextNames::ExprIfCond,
                ContextNames::ExprIfThen,
                ContextNames::ExprIfElse
            },
            {
                ContextOverride{.context_name = ContextNames::ExprSingleAssignment},
                ContextOverride{.context_name = ContextNames::ExprMultiAssignment},
                ContextOverride{.context_name = ContextNames::ExprReassignment},
                ContextOverride{.context_name = ContextNames::ExprReturnStmt},
                ContextOverride{.context_name = ContextNames::ExprDirectiveNoEq},
                ContextOverride{.context_name = ContextNames::ExprDirectiveEq},
                ContextOverride{.context_name = ContextNames::ExprEnumCase},
                ContextOverride{.context_name = ContextNames::ExprDictValue},
                ContextOverride{.context_name = ContextNames::ExprDictValueFirst},
                ContextOverride{.context_name = ContextNames::ExprDictValueComma},
                ContextOverride{.context_name = ContextNames::ExprSwitchCase}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "(1\n+ 2)\n+ 3\n",
            "MultilineBinaryRejectedAfterGroupingCloses",
            {
                ContextNames::ExprIfCond,
                ContextNames::ExprIfThen,
                ContextNames::ExprIfElse
            },
            {
                ContextOverride{.context_name = ContextNames::ExprSingleAssignment},
                ContextOverride{.context_name = ContextNames::ExprMultiAssignment},
                ContextOverride{.context_name = ContextNames::ExprReassignment},
                ContextOverride{.context_name = ContextNames::ExprReturnStmt},
                ContextOverride{.context_name = ContextNames::ExprDirectiveNoEq},
                ContextOverride{.context_name = ContextNames::ExprDirectiveEq},
                ContextOverride{.context_name = ContextNames::ExprEnumCase},
                ContextOverride{.context_name = ContextNames::ExprDictValue},
                ContextOverride{.context_name = ContextNames::ExprDictValueFirst},
                ContextOverride{.context_name = ContextNames::ExprDictValueComma},
                ContextOverride{.context_name = ContextNames::ExprSwitchCase}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1\n*",
            "DanglingBinaryOperatorAtNewline",
            {
                ContextNames::ExprIfCond,
                ContextNames::ExprIfThen,
                ContextNames::ExprIfElse
            },
            {
                ContextOverride{.context_name = ContextNames::ExprEnumCase},
                ContextOverride{.context_name = ContextNames::ExprDictValue},
                ContextOverride{.context_name = ContextNames::ExprDictValueFirst},
                ContextOverride{.context_name = ContextNames::ExprDictValueComma},
                ContextOverride{.context_name = ContextNames::ExprSwitchCase}
            }
        );
    }
#endif
}
