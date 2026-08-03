#include "frontend/parser/helpers/context_names.h"
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "BinaryMissingRight",
                .code = "1 + ",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 0, .column_start = 0, .line_end = 0,
                        .column_end = 0, .skip_span_check = true
                    }
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryMissingLeft",
                .code = "* 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 2
                    }
                },
                .verifier = IsNull()
            });

            reg({
                .name = "BinaryInvalidRight1",
                .code = "1 + * 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    }
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight2",
                .code = "1 + *",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    }
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight3",
                .code = "1 + - * 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsBinary(TokenType::Plus,
                                     IsNumber("1"),
                                     IsUnary(TokenType::Minus,
                                             IsNull())
                )
            });

            reg({
                .name = "UnaryInvalidRight1",
                .code = "+ *",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4
                    }
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryInvalidRight2",
                .code = "+ * 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4
                    }
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryInvalidRight3",
                .code = "- + * 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    }
                },
                .verifier = IsUnary(TokenType::Minus,
                                    IsUnary(TokenType::Plus,
                                            IsNull()
                                    )
                )
            });

            reg({
                .name = "UnaryInvalidRight4",
                .code = "+ .",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4
                    }
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryMissingRight1",
                .code = "- ",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 0, .column_start = 0, .line_end = 0,
                        .column_end = 0, .skip_span_check = true
                    }
                },
                .verifier = IsUnary(TokenType::Minus, IsNull())
            });

            reg({
                .name = "UnaryMissingRight2",
                .code = "not ",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 0, .column_start = 0, .line_end = 0,
                        .column_end = 0, .skip_span_check = true
                    }
                },
                .verifier = IsUnary(TokenType::Not, IsNull())
            });

            reg({
                .name = "RightAssociativeMissingOperand",
                .code = "2 ^ ^ 3 ",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6
                    }
                },
                .verifier = IsBinary(TokenType::Caret, IsNumber("2"), IsNull())
            });

            reg({
                .name = "ModifierInsideExpressionContext",
                .code = "1 + @test 2",
                .errors = {
                    PErr{
                        .code = E::ModifiersAttachedToInvalidDeclaration, .line_start = 1, .column_start = 5,
                        .line_end = 1, .column_end = 10
                    }
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))
            });

            reg({
                .name = "MultilineBinary",
                .code = "1\n* 2\n",
                .errors = {},
                .verifier = IsBinary(TokenType::Star, IsNumber("1"), IsNumber("2")),
                .skip_contexts = {
                    ContextNames::ExprIfCond,
                    ContextNames::ExprIfThen,
                    ContextNames::ExprIfElse
                },
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSingleAssignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2,
                                .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprMultiAssignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2,
                                .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprReassignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2,
                                .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprReturnStmt,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2,
                                .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDirectiveNoEq,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2,
                                .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDirectiveEq,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2,
                                .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprEnumCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1,
                                .column_end = 3
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValue,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1,
                                .column_end = 3
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueFirst,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1,
                                .column_end = 3
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueComma,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1,
                                .column_end = 3
                            }
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 2, .column_start = 1,
                                .line_end = 2, .column_end = 2
                            }
                        },
                        .verifier = IsNumber("1")
                    }
                }
            });

            reg({
                .name = "MultilineBinaryRejectedAfterGroupingCloses",
                .code = "(1\n+ 2)\n+ 3\n",
                .errors = {},
                .verifier = IsBinary(
                    TokenType::Plus,
                    IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))),
                    IsNumber("3")
                ),
                .skip_contexts = {
                    ContextNames::ExprIfCond,
                    ContextNames::ExprIfThen,
                    ContextNames::ExprIfElse
                },
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSingleAssignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidStandaloneStatement, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 4}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprMultiAssignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidStandaloneStatement, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 4}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprReassignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidStandaloneStatement, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 4}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprReturnStmt,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidStandaloneStatement, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 4}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDirectiveNoEq,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidStandaloneStatement, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 4}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDirectiveEq,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidStandaloneStatement, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 4}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprEnumCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 2, .column_start = 5, .line_end = 2, .column_end = 6}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValue,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 2, .column_start = 5, .line_end = 2, .column_end = 6}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueFirst,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 2, .column_start = 5, .line_end = 2, .column_end = 6}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueComma,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 2, .column_start = 5, .line_end = 2, .column_end = 6}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 3, .column_start = 1, .line_end = 3, .column_end = 2}
                        },
                        .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))
                    }
                }
            });

            reg({
                .name = "DanglingBinaryOperatorAtNewline",
                .code = "1\n*",
                .errors = std::vector<ParserExpectedError>{
                    PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 2, .line_end = 2, .column_end = 3}
                },
                .verifier = IsBinary(TokenType::Star, IsNumber("1")),
                .skip_contexts = {
                    ContextNames::ExprIfCond,
                    ContextNames::ExprIfThen,
                    ContextNames::ExprIfElse
                },
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSingleAssignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprMultiAssignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprReassignment,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprReturnStmt,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDirectiveNoEq,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDirectiveEq,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprEnumCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValue,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueFirst,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprDictValueComma,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}
                        },
                        .verifier = IsNumber("1")
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprSwitchCase,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedCaseOrDefaultInsideSwitchBody, .line_start = 2, .column_start = 1, .line_end = 2, .column_end = 2}
                        },
                        .verifier = IsNumber("1")
                    }
                }
            });

            return true;
        }();
    }
}
