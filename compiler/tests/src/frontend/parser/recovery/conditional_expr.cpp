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
                .name = "InvalidConditionExpression",
                .code = "if * then 1 else 1",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5
                    }
                },
                .verifier = IsConditional(IsNull(), IsNumber("1"), IsNumber("1"))
            });

            reg({
                .name = "InvalidThenBranchExpression",
                .code = "if 1 then * else 1",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 11, .line_end = 1,
                        .column_end = 12
                    }
                },
                .verifier = IsConditional(IsNumber("1"), IsNull(), IsNumber("1"))
            });

            reg({
                .name = "InvalidElseBranchExpression",
                .code = "if 1 then 1 else *",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 18, .line_end = 1,
                        .column_end = 19
                    }
                },
                .verifier = IsConditional(IsNumber("1"), IsNumber("1"), IsNull())
            });

            reg({
                .name = "MissingThenTokenRecoversBranches",
                .code = "if x 1 else 2",
                .errors = {
                    PErr{
                        .code = E::MissingOperator, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7
                    },
                    PErr{
                        .code = E::MissingThenToken, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsConditional(IsBinary(TokenType::Error, IsIdentifier("x"), IsNumber("1")), IsNull(),
                                          IsNumber("2"))
            });

            reg({
                .name = "MissingElseEntirelyNoExpression",
                .code = "if x then 1",
                .errors = {
                    PErr{
                        .code = E::MissingElseToken, .line_start = 1, .column_start = 12, .line_end = 1,
                        .column_end = 13
                    }
                },
                .verifier = IsConditional(IsIdentifier("x"), IsNumber("1"), IsNull()),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 20}
                        },
                        .verifier = IsConditional(IsNumber("1"), IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2")), IsNull()),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                }
            });

            reg({
                .name = "DanglingElseStealsFromOuterIf",
                .code = "if 1 then if x then 1 else 2",
                .errors = {
                    PErr{
                        .code = E::MissingElseToken, .line_start = 1, .column_start = 29, .line_end = 1,
                        .column_end = 30
                    }
                },
                .verifier = IsConditional(
                    IsNumber("1"),
                    IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2")),
                    IsNull()
                ),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 36, .line_end = 1, .column_end = 37}
                        },
                        .verifier = IsConditional(
                            IsNumber("1"),
                            IsConditional(
                                IsNumber("1"),
                                IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2")),
                                IsNumber("2")
                            ),
                            IsNull()
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                }
            });

            reg({
                .name = "MissingElseTokenRecoversBranches",
                .code = "if x then 1 2",
                .errors = {
                    PErr{
                        .code = E::MissingOperator, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14
                    },
                    PErr{
                        .code = E::MissingElseToken, .line_start = 1, .column_start = 14, .line_end = 1,
                        .column_end = 15
                    }
                },
                .verifier = IsConditional(IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")),
                                          IsNull()),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfCond,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14},
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsConditional(IsConditional(IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")), IsNull()), IsNumber("1"), IsNumber("2")),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14},
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}
                        },
                        .verifier = IsConditional(IsNumber("1"), IsConditional(IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")), IsNumber("2")), IsNull()),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                }
            });

            reg({
                .name = "MissingBothThenAndElseTokens",
                .code = "if x 1 2",
                .errors = {
                    PErr{
                        .code = E::MissingOperator, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7
                    },
                    PErr{
                        .code = E::MissingOperator, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9
                    },
                    PErr{
                        .code = E::MissingThenToken, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10
                    },
                    PErr{
                        .code = E::MissingElseToken, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10
                    }
                },
                .verifier = IsConditional(IsBinary(TokenType::Error, IsIdentifier("x"),
                                                   IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2"))), IsNull(),
                                          IsNull()),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfCond,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::MissingThenToken, .line_start = 1, .column_start = 23, .line_end = 1, .column_end = 24},
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 23, .line_end = 1, .column_end = 24}
                        },
                        .verifier = IsConditional(
                            IsConditional(
                                IsBinary(TokenType::Error, IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2"))),
                                IsNumber("1"),
                                IsNumber("2")
                            ),
                            IsNull(),
                            IsNull()
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7},
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::MissingThenToken, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 17}
                        },
                        .verifier = IsConditional(
                            IsNumber("1"),
                            IsConditional(
                                IsBinary(TokenType::Error, IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2"))),
                                IsNull(),
                                IsNumber("2")
                            ),
                            IsNull()
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                }
            });

            reg({
                .name = "EmptyConditionRecoversThenAndElse",
                .code = "if then 1 else 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsConditional(IsNull(), IsNumber("1"), IsNumber("2"))
            });

            reg({
                .name = "EmptyThenBranchRecoversElse",
                .code = "if x then else 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 11, .line_end = 1,
                        .column_end = 15
                    }
                },
                .verifier = IsConditional(IsIdentifier("x"), IsNull(), IsNumber("2"))
            });

            reg({
                .name = "MissingThenAndElseTokensMissingOperatosInCond",
                .code = "if x > 5 y * 2 z - 3",
                .errors = {
                    PErr{
                        .code = E::MissingOperator, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11
                    },
                    PErr{
                        .code = E::MissingOperator, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 17
                    },
                    PErr{
                        .code = E::MissingThenToken, .line_start = 1, .column_start = 21, .line_end = 1,
                        .column_end = 22
                    },
                    PErr{
                        .code = E::MissingElseToken, .line_start = 1, .column_start = 21, .line_end = 1,
                        .column_end = 22
                    }
                },
                .verifier = IsConditional(
                    IsBinary(TokenType::Greater,
                             IsIdentifier("x"),
                             IsBinary(TokenType::Minus,
                                      IsBinary(TokenType::Star,
                                               IsBinary(TokenType::Error, IsNumber("5"), IsIdentifier("y")),
                                               IsBinary(TokenType::Error, IsNumber("2"), IsIdentifier("z"))
                                      ),
                                      IsNumber("3")
                             )
                    ),
                    IsNull(),
                    IsNull()
                ),
                .context_overrides = {
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfCond,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11},
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 17},
                            PErr{.code = E::MissingThenToken, .line_start = 1, .column_start = 35, .line_end = 1, .column_end = 36},
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 35, .line_end = 1, .column_end = 36}
                        },
                        .verifier = IsConditional(
                            IsConditional(
                                IsBinary(TokenType::Greater,
                                         IsIdentifier("x"),
                                         IsBinary(TokenType::Minus,
                                                  IsBinary(TokenType::Star,
                                                           IsBinary(TokenType::Error, IsNumber("5"), IsIdentifier("y")),
                                                           IsBinary(TokenType::Error, IsNumber("2"), IsIdentifier("z"))
                                                  ),
                                                  IsNumber("3")
                                         )
                                ),
                                IsNumber("1"),
                                IsNumber("2")
                            ),
                            IsNull(),
                            IsNull()
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    },
                    ContextOverride<ExprVerifier>{
                        .context_name = ContextNames::ExprIfThen,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11},
                            PErr{.code = E::MissingOperator, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 17},
                            PErr{.code = E::MissingThenToken, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22},
                            PErr{.code = E::MissingElseToken, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 29}
                        },
                        .verifier = IsConditional(
                            IsNumber("1"),
                            IsConditional(
                                IsBinary(TokenType::Greater,
                                         IsIdentifier("x"),
                                         IsBinary(TokenType::Minus,
                                                  IsBinary(TokenType::Star,
                                                           IsBinary(TokenType::Error, IsNumber("5"), IsIdentifier("y")),
                                                           IsBinary(TokenType::Error, IsNumber("2"), IsIdentifier("z"))
                                                  ),
                                                  IsNumber("3")
                                         )
                                ),
                                IsNull(),
                                IsNumber("2")
                            ),
                            IsNull()
                        ),
                        .skip_after_depth_0 = true,
                        .skip_transform = true
                    }
                }
            });

            reg({
                .name = "GarbageInConditionAndThenBranchRecoversElse",
                .code = "if * then * else 2",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5
                    },
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 11, .line_end = 1,
                        .column_end = 12
                    }
                },
                .verifier = IsConditional(IsNull(), IsNull(), IsNumber("2"))
            });

            return true;
        }();
    }
}
