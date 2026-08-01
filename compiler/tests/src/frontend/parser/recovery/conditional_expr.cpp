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
                    {E::InvalidExpression, 1, 4, 1, 5}
                },
                .verifier = IsConditional(IsNull(), IsNumber("1"), IsNumber("1"))
            });

            reg({
                .name = "InvalidThenBranchExpression",
                .code = "if 1 then * else 1",
                .errors = {
                    {E::InvalidExpression, 1, 11, 1, 12}
                },
                .verifier = IsConditional(IsNumber("1"), IsNull(), IsNumber("1"))
            });

            reg({
                .name = "InvalidElseBranchExpression",
                .code = "if 1 then 1 else *",
                .errors = {
                    {E::InvalidExpression, 1, 18, 1, 19}
                },
                .verifier = IsConditional(IsNumber("1"), IsNumber("1"), IsNull())
            });

            reg({
                .name = "MissingThenTokenRecoversBranches",
                .code = "if x 1 else 2",
                .errors = {
                    {E::MissingOperator, 1, 6, 1, 7},
                    {E::MissingThenToken, 1, 7, 1, 8}
                },
                .verifier = IsConditional(IsBinary(TokenType::Error, IsIdentifier("x"), IsNumber("1")), IsNull(), IsNumber("2")),
                .skip_contexts = {ContextNames::ExprIfCond, ContextNames::ExprIfThen}
            });

            reg({
                .name = "MissingElseEntirelyNoExpression",
                .code = "if x then 1",
                .errors = {
                    {E::MissingElseToken, 1, 12, 1, 13}
                },
                .verifier = IsConditional(IsIdentifier("x"), IsNumber("1"), IsNull()),
                .skip_contexts = {ContextNames::ExprIfThen}
            });

            reg({
                .name = "DanglingElseStealsFromOuterIf",
                .code = "if 1 then if x then 1 else 2",
                .errors = {
                    {E::MissingElseToken, 1, 29, 1, 30}
                },
                .verifier = IsConditional(
                    IsNumber("1"),
                    IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2")),
                    IsNull()
                ),
                .skip_contexts = {ContextNames::ExprIfThen}
            });

            reg({
                .name = "MissingElseTokenRecoversBranches",
                .code = "if x then 1 2",
                .errors = {
                    {E::MissingOperator, 1, 13, 1, 14},
                    {E::MissingElseToken, 1, 14, 1, 15}
                },
                .verifier = IsConditional(IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")), IsNull()),
                .skip_contexts = {ContextNames::ExprIfCond, ContextNames::ExprIfThen}
            });

            reg({
                .name = "MissingBothThenAndElseTokens",
                .code = "if x 1 2",
                .errors = {
                    {E::MissingOperator, 1, 6, 1, 7},
                    {E::MissingOperator, 1, 8, 1, 9},
                    {E::MissingThenToken, 1, 9, 1, 10},
                    {E::MissingElseToken, 1, 9, 1, 10}
                },
                .verifier = IsConditional(IsBinary(TokenType::Error, IsIdentifier("x"), IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2"))), IsNull(), IsNull()),
                .skip_contexts = {ContextNames::ExprIfCond, ContextNames::ExprIfThen}
            });

            reg({
                .name = "EmptyConditionRecoversThenAndElse",
                .code = "if then 1 else 2",
                .errors = {
                    {E::InvalidExpression, 1, 4, 1, 8}
                },
                .verifier = IsConditional(IsNull(), IsNumber("1"), IsNumber("2"))
            });

            reg({
                .name = "EmptyThenBranchRecoversElse",
                .code = "if x then else 2",
                .errors = {
                    {E::InvalidExpression, 1, 11, 1, 15}
                },
                .verifier = IsConditional(IsIdentifier("x"), IsNull(), IsNumber("2"))
            });

            reg({
                .name = "MissingThenAndElseTokensButValidExpressionsMaintainsAstIntegrity",
                .code = "if x > 5 y * 2 z - 3",
                .errors = {
                    {E::MissingOperator, 1, 10, 1, 11},
                    {E::MissingOperator, 1, 16, 1, 17},
                    {E::MissingThenToken, 1, 21, 1, 22},
                    {E::MissingElseToken, 1, 21, 1, 22}
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
                .skip_contexts = {ContextNames::ExprIfCond, ContextNames::ExprIfThen}
            });

            reg({
                .name = "GarbageInConditionAndThenBranchRecoversElse",
                .code = "if * then * else 2",
                .errors = {
                    {E::InvalidExpression, 1, 4, 1, 5},
                    {E::InvalidExpression, 1, 11, 1, 12}
                },
                .verifier = IsConditional(IsNull(), IsNull(), IsNumber("2"))
            });

            return true;
        }();
    }
}
