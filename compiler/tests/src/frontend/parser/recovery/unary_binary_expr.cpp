#include "frontend/parser/helpers/context_names.h"
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v,
                          const std::vector<std::string_view>& skip_contexts = {})
            {
                ErrorRegistry::add(n, c, errs, v, skip_contexts);
            };

            reg("BinaryMissingRight", "1 + ",
                {
                    {E::InvalidExpression, 0, 0, 0, 0, true}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            );

            reg("BinaryInvalidRight1", "1 + * 2",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            );

            reg("BinaryInvalidRight2", "1 + *",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            );

            reg("BinaryInvalidRight3", "1 + - * 2",
                {
                    {E::InvalidExpression, 1, 7, 1, 8}
                },
                IsBinary(TokenType::Plus,
                         IsNumber("1"),
                         IsUnary(TokenType::Minus,
                                 IsNull())
                ));

            reg("UnaryInvalidRight1", "+ *",
                {
                    {E::InvalidExpression, 1, 3, 1, 4}
                },
                IsUnary(TokenType::Plus, IsNull())
            );

            reg("UnaryInvalidRight2", "+ * 2",
                {
                    {E::InvalidExpression, 1, 3, 1, 4}
                },
                IsUnary(TokenType::Plus, IsNull())
            );

            reg("UnaryInvalidRight3", "- + * 2",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsUnary(TokenType::Minus,
                        IsUnary(TokenType::Plus,
                                IsNull()
                        )
                ));

            reg("UnaryInvalidRight4", "+ .",
                {
                    {E::InvalidExpression, 1, 3, 1, 4}
                },
                IsUnary(TokenType::Plus, IsNull())
            );

            reg("UnaryMissingRight1", "- ",
                {
                    {E::InvalidExpression, 0, 0, 0, 0, true}
                },
                IsUnary(TokenType::Minus, IsNull())
            );

            reg("UnaryMissingRight2", "not ",
                {
                    {E::InvalidExpression, 0, 0, 0, 0, true}
                },
                IsUnary(TokenType::Not, IsNull())
            );

            reg("RightAssociativeMissingOperand", "2 ^ ^ 3 ",
                {
                    {E::InvalidExpression, 1, 5, 1, 6}
                },
                IsBinary(TokenType::Caret, IsNumber("2"), IsNull())
            );

            reg("ModifierInsideExpressionContext", "1 + @test 2",
                {
                    {E::TopLevelDeclarationNotAllowedHere, 1, 5, 1, 10},
                    {E::ModifiersAttachedToInvalidDeclaration, 1, 5, 1, 10}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))
            );

            return true;
        }();
    }
}
