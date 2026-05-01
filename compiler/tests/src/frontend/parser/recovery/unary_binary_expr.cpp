#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& errs, const auto& v) { ErrorRegistry::add(n, c, errs, v); };

            reg("BinaryMissingRight", "1 + ",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 0, 0, 0, 0, true}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            );

            reg("BinaryInvalidRight1", "1 + * 2",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 5, 1, 6}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            );

            reg("BinaryInvalidRight2", "1 + *",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 5, 1, 6}
                },
                IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            );

            reg("BinaryInvalidRight3", "1 + - * 2",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 7, 1, 8}
                },
                IsBinary(TokenType::Plus,
                         IsNumber("1"),
                         IsUnary(TokenType::Minus,
                                 IsNull())
                ));

            reg("UnaryInvalidRight1", "+ *",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 3, 1, 4}
                },
                IsUnary(TokenType::Plus, IsNull())
            );

            reg("UnaryInvalidRight2", "+ * 2",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 3, 1, 4}
                },
                IsUnary(TokenType::Plus, IsNull())
            );

            reg("UnaryInvalidRight3", "- + * 2",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 5, 1, 6}
                },
                IsUnary(TokenType::Minus,
                        IsUnary(TokenType::Plus,
                                IsNull()
                        )
                ));

            reg("UnaryInvalidRight4", "+ .",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 3, 1, 4}
                },
                IsUnary(TokenType::Plus, IsNull())
            );

            reg("UnaryMissingRight1", "- ",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 0, 0, 0, 0, true}
                },
                IsUnary(TokenType::Minus, IsNull())
            );

            reg("UnaryMissingRight2", "not ",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 0, 0, 0, 0, true}
                },
                IsUnary(TokenType::Not, IsNull())
            );

            reg("RightAssociativeMissingOperand", "2 ^ ^ 3 ",
                std::vector<ExpectedError>{
                    {ValuascriptErrorCode::InvalidExpression, 1, 5, 1, 6}
                },
                IsBinary(TokenType::Caret, IsNumber("2"), IsNull())
            );

            return true;
        }();
    }
}
