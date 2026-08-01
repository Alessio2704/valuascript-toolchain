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
                    {E::InvalidExpression, 0, 0, 0, 0, true}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight1",
                .code = "1 + * 2",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight2",
                .code = "1 + *",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight3",
                .code = "1 + - * 2",
                .errors = {
                    {E::InvalidExpression, 1, 7, 1, 8}
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
                    {E::InvalidExpression, 1, 3, 1, 4}
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryInvalidRight2",
                .code = "+ * 2",
                .errors = {
                    {E::InvalidExpression, 1, 3, 1, 4}
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryInvalidRight3",
                .code = "- + * 2",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
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
                    {E::InvalidExpression, 1, 3, 1, 4}
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryMissingRight1",
                .code = "- ",
                .errors = {
                    {E::InvalidExpression, 0, 0, 0, 0, true}
                },
                .verifier = IsUnary(TokenType::Minus, IsNull())
            });

            reg({
                .name = "UnaryMissingRight2",
                .code = "not ",
                .errors = {
                    {E::InvalidExpression, 0, 0, 0, 0, true}
                },
                .verifier = IsUnary(TokenType::Not, IsNull())
            });

            reg({
                .name = "RightAssociativeMissingOperand",
                .code = "2 ^ ^ 3 ",
                .errors = {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                .verifier = IsBinary(TokenType::Caret, IsNumber("2"), IsNull())
            });

            reg({
                .name = "ModifierInsideExpressionContext",
                .code = "1 + @test 2",
                .errors = {
                    {E::TopLevelDeclarationNotAllowedHere, 1, 5, 1, 10},
                    {E::ModifiersAttachedToInvalidDeclaration, 1, 5, 1, 10}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))
            });

            return true;
        }();
    }
}
