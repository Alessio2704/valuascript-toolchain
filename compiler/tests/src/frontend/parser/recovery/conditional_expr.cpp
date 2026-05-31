#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v,
                          const std::vector<std::string_view>& skip = {})
            {
                ErrorRegistry::add(n, c, errs, v, skip);
            };

            reg("InvalidConditionExpression",
                "if * then 1 else 1",
                {
                    {E::InvalidExpression, 1, 4, 1, 5}
                },
                IsConditional(IsNull(), IsNumber("1"), IsNumber("1"))
            );

            reg("InvalidThenBranchExpression",
                "if 1 then * else 1",
                {
                    {E::InvalidExpression, 1, 11, 1, 12}
                },
                IsConditional(IsNumber("1"), IsNull(), IsNumber("1"))
            );

            reg("InvalidElseBranchExpression",
                "if 1 then 1 else *",
                {
                    {E::InvalidExpression, 1, 18, 1, 19}
                },
                IsConditional(IsNumber("1"), IsNumber("1"), IsNull())
            );

            reg("MissingThenTokenRecoversBranches",
                "if x 1 else 2",
                {
                    {E::MissingThenToken, 1, 5, 1, 6}
                },
                IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2"))
            );

            reg("MissingElseEntirelyNoExpression",
                "if x then 1",
                {
                    {E::MissingElseToken, 1, 12, 1, 13},
                },
                IsConditional(
                    IsIdentifier("x"), IsNumber("1"), IsNull()
                ),
                {ContextNames::ExprIfThen}
            );

            reg("DanglingElseStealsFromOuterIf",
                "if 1 then if x then 1 else 2",
                {
                    {E::MissingElseToken, 1, 29, 1, 30}
                },
                IsConditional(
                    IsNumber("1"),
                    IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2")),
                    IsNull()
                ),
                {ContextNames::ExprIfThen}
            );

            reg("MissingElseTokenRecoversBranches",
                "if x then 1 2",
                {
                    {E::MissingElseToken, 1, 12, 1, 13}
                },
                IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2"))
            );

            reg("MissingBothThenAndElseTokens",
                "if x 1 2",
                {
                    {E::MissingThenToken, 1, 5, 1, 6},
                    {E::MissingElseToken, 1, 7, 1, 8}
                },
                IsConditional(IsIdentifier("x"), IsNumber("1"), IsNumber("2"))
            );

            reg("EmptyConditionRecoversThenAndElse",
                "if then 1 else 2",
                {
                    {E::InvalidExpression, 1, 4, 1, 8}
                },
                IsConditional(IsNull(), IsNumber("1"), IsNumber("2"))
            );

            reg("EmptyThenBranchRecoversElse",
                "if x then else 2",
                {
                    {E::InvalidExpression, 1, 11, 1, 15}
                },
                IsConditional(IsIdentifier("x"), IsNull(), IsNumber("2"))
            );

            reg("MissingThenAndElseTokensButValidExpressionsMaintainsAstIntegrity",
                "if x > 5 y * 2 z - 3",
                {
                    {E::MissingThenToken, 1, 9, 1, 10},
                    {E::MissingElseToken, 1, 15, 1, 16}
                },
                IsConditional(
                    IsBinary(TokenType::Greater, IsIdentifier("x"), IsNumber("5")),
                    IsBinary(TokenType::Star, IsIdentifier("y"), IsNumber("2")),
                    IsBinary(TokenType::Minus, IsIdentifier("z"), IsNumber("3"))
                )
            );

            reg("GarbageInConditionAndThenBranchRecoversElse",
                "if * then * else 2",
                {
                    {E::InvalidExpression, 1, 4, 1, 5},
                    {E::InvalidExpression, 1, 11, 1, 12}
                },
                IsConditional(IsNull(), IsNull(), IsNumber("2"))
            );

            return true;
        }();
    }
}
