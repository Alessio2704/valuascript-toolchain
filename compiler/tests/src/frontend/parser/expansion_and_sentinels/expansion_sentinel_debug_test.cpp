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
                ContextNames::TypeGenericTypeStart,
                ContextNames::TypeGenericTypeMiddle,
                ContextNames::TypeGenericTypeEnd,
                ContextNames::TypeGenericTypeSingle
            },
            {
                ContextOverride{.context_name = ContextNames::TypeFunctionReturnStart},
                ContextOverride{.context_name = ContextNames::TypeFunctionReturnMiddle},
                ContextOverride{.context_name = ContextNames::TypeMultiAssignmentTarget1},
                ContextOverride{.context_name = ContextNames::TypeFunctionReturnSingle},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeSingle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::TypeGenericTypeEnd, .skip_after_depth_0 = true}
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
                ContextNames::TypeTupleTypeSingle,
                ContextNames::TypeGenericTypeStart,
                ContextNames::TypeGenericTypeMiddle,
                ContextNames::TypeGenericTypeEnd,
                ContextNames::TypeGenericTypeSingle,
                ContextNames::TypeFunctionParamStart,
                ContextNames::TypeFunctionParamMiddle,
                ContextNames::TypeFunctionParamEnd,
                ContextNames::TypeFunctionReturnStart,
                ContextNames::TypeFunctionReturnMiddle,
                ContextNames::TypeFunctionReturnEnd
            },
            {
                ContextOverride{.context_name = ContextNames::TypeFunctionParamSingle},
                ContextOverride{.context_name = ContextNames::TypeFunctionParamMiddle},
                ContextOverride{.context_name = ContextNames::TypeTupleTypeEnd},
                ContextOverride{.context_name = ContextNames::TypeMultiAssignmentTarget1},
                ContextOverride{.context_name = ContextNames::TypeFunctionReturnStart},
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
            {},
            {
                ContextOverride{.context_name = ContextNames::ExprSingleAssignment},
                ContextOverride{.context_name = ContextNames::ExprMultiAssignment},
                ContextOverride{.context_name = ContextNames::ExprReassignment},
                ContextOverride{.context_name = ContextNames::ExprReturnStmt},
                ContextOverride{.context_name = ContextNames::ExprDirectiveNoEq},
                ContextOverride{.context_name = ContextNames::ExprDirectiveEq},
                ContextOverride{.context_name = ContextNames::ExprEnumCase},
                ContextOverride{.context_name = ContextNames::ExprDictValueStart},
                ContextOverride{.context_name = ContextNames::ExprDictValueMiddle},
                ContextOverride{.context_name = ContextNames::ExprDictValueEnd},
                ContextOverride{.context_name = ContextNames::ExprDictValueSingle},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseStart},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseMiddle},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseEnd},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseSingle}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "(1\n+ 2)\n+ 3\n",
            "MultilineBinaryRejectedAfterGroupingCloses",
            {},
            {
                ContextOverride{.context_name = ContextNames::ExprSingleAssignment},
                ContextOverride{.context_name = ContextNames::ExprMultiAssignment},
                ContextOverride{.context_name = ContextNames::ExprReassignment},
                ContextOverride{.context_name = ContextNames::ExprReturnStmt},
                ContextOverride{.context_name = ContextNames::ExprDirectiveNoEq},
                ContextOverride{.context_name = ContextNames::ExprDirectiveEq},
                ContextOverride{.context_name = ContextNames::ExprEnumCase},
                ContextOverride{.context_name = ContextNames::ExprDictValueStart},
                ContextOverride{.context_name = ContextNames::ExprDictValueMiddle},
                ContextOverride{.context_name = ContextNames::ExprDictValueEnd},
                ContextOverride{.context_name = ContextNames::ExprDictValueSingle},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseStart},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseMiddle},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseEnd},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseSingle}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1\n*",
            "DanglingBinaryOperatorAtNewline",
            {},
            {
                ContextOverride{.context_name = ContextNames::ExprIfCond},
                ContextOverride{.context_name = ContextNames::ExprIfThen},
                ContextOverride{.context_name = ContextNames::ExprIfElse},
                ContextOverride{.context_name = ContextNames::ExprSingleAssignment},
                ContextOverride{.context_name = ContextNames::ExprMultiAssignment},
                ContextOverride{.context_name = ContextNames::ExprReassignment},
                ContextOverride{.context_name = ContextNames::ExprReturnStmt},
                ContextOverride{.context_name = ContextNames::ExprDirectiveNoEq},
                ContextOverride{.context_name = ContextNames::ExprDirectiveEq},
                ContextOverride{.context_name = ContextNames::ExprEnumCase},
                ContextOverride{.context_name = ContextNames::ExprDictValueStart},
                ContextOverride{.context_name = ContextNames::ExprDictValueMiddle},
                ContextOverride{.context_name = ContextNames::ExprDictValueEnd},
                ContextOverride{.context_name = ContextNames::ExprDictValueSingle},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseStart},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseMiddle},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseEnd},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseSingle}
            }
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "[1, 2, 3",
            "UnclosedTensorLiteral",
            {},
            {
                ContextOverride{.context_name = ContextNames::ExprBracketAccessIndex, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTensorStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTensorMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTensorEnd, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTensorSingle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprDictValueStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprDictValueMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprDictValueEnd, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprDictValueSingle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprCallArgStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprCallArgMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprCallArgEnd, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprCallArgSingle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseEnd, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprSwitchCaseSingle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprGrouping, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprUnaryGrouping, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTupleStart, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTupleMiddle, .skip_after_depth_0 = true},
                ContextOverride{.context_name = ContextNames::ExprTupleEnd, .skip_after_depth_0 = true}
            }
        );
    }
#endif
}
