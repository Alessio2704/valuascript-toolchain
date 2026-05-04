#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExprVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("ChainingNotAllowedForComparisonOperations1", "1 < 2 < 3",
                {
                    {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 7, 1, 8}
                },
                IsBinary(TokenType::Less,
                         IsBinary(TokenType::Less,
                                  IsNumber("1"),
                                  IsNumber("2")
                         ),
                         IsNumber("3")
                ));

            reg("ChainingNotAllowedForComparisonOperations2", "1 > 2 > 3",
                {
                    {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 7, 1, 8}
                },
                IsBinary(TokenType::Greater,
                         IsBinary(TokenType::Greater,
                                  IsNumber("1"),
                                  IsNumber("2")
                         ),
                         IsNumber("3")
                ));

            reg("ChainingNotAllowedForComparisonOperations3", "1 != 2 != 3 ",
                {
                    {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 8, 1, 10}
                },
                IsBinary(TokenType::NotEquals,
                         IsBinary(TokenType::NotEquals,
                                  IsNumber("1"),
                                  IsNumber("2")
                         ),
                         IsNumber("3")
                ));

            reg("ChainingNotAllowedForComparisonOperations4", "x == y == z",
                {
                    {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 8, 1, 10}
                },
                IsBinary(TokenType::Equals,
                         IsBinary(TokenType::Equals,
                                  IsIdentifier("x"),
                                  IsIdentifier("y")
                         ),
                         IsIdentifier("z")
                )
            );

            reg("ChainingNotAllowedForComparisonOperationsMixedOperators1", "1 < 2 > 3",
                {
                    {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 7, 1, 8}
                },
                IsBinary(TokenType::Greater,
                         IsBinary(TokenType::Less,
                                  IsNumber("1"),
                                  IsNumber("2")
                         ),
                         IsNumber("3")
                )
            );

            reg("ChainingNotAllowedForComparisonOperationsMixedOperators2", "1 == 2 != 3",
                {
                    {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 8, 1, 10}
                },
                IsBinary(TokenType::NotEquals,
                         IsBinary(TokenType::Equals,
                                  IsNumber("1"),
                                  IsNumber("2")
                         ),
                         IsNumber("3")
                ));

            return true;
        }();
    }
}
