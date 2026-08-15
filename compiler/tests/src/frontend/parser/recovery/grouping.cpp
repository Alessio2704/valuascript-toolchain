#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "GroupingMissingClosingParen",
                .code = "( 1 + 2",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightParenAfterExpression,
                        .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = IsGrouping(IsTuple(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTupleMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                        },
                        .verifier = IsTuple(
                            IsNumber("1"),
                            IsTuple(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")), IsNumber("3"))
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = IsTensor(IsTuple(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")), IsNumber("2"), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprTensorMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                        },
                        .verifier = IsTensor(IsNumber("1"), IsTuple(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")), IsNumber("3"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprGrouping, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprUnaryGrouping, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTupleEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprTensorSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprDictValueSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprCallArgSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBinaryLhs,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), IsNumber("100"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprBinaryRhs,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsBinary(TokenType::Plus, IsNumber("100"), IsGrouping(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsCallTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterArguments, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}
                        },
                        .verifier = IsGrouping(IsCall(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsDotTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = IsGrouping(IsDot(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), "prop")),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsBracketTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}
                        },
                        .verifier = IsGrouping(IsBracket(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), IsNumber("0"))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprAsSliceTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsGrouping(IsBracket(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10")))),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCond,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsSwitch(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), std::vector<SwitchCaseSpec>{}, IsNumber("1")),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfCond,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsConditional(IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), IsNumber("1"), IsNumber("2")),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        },
                        .verifier = IsConditional(IsNumber("1"), IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))), IsNumber("2")),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseStart, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseMiddle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseEnd, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprSwitchCaseSingle, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprFuncDefDefault, .skip_after_depth_0 = true},
                    ContextOverride<ExprVerifier>{.context_name = ContextNames::ExprModifierArg, .skip_after_depth_0 = true}
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            return true;
        }();
    }
}
