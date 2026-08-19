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
                .name = "ChainingNotAllowedForComparisonOperations1",
                .code = "1 < 2 < 3",
                .errors = {
                    PErr{.code = E::ChainingNotAllowedForComparisonOperations, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBinary(
                    TokenType::Less,
                    IsBinary(TokenType::Less, IsNumber("1"), IsNumber("2")),
                    IsNumber("3")
                )
            });

            reg({
                .name = "ChainingNotAllowedForComparisonOperations2",
                .code = "1 > 2 > 3",
                .errors = {
                    PErr{.code = E::ChainingNotAllowedForComparisonOperations, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBinary(
                    TokenType::Greater,
                    IsBinary(TokenType::Greater, IsNumber("1"), IsNumber("2")),
                    IsNumber("3")
                )
            });

            reg({
                .name = "ChainingNotAllowedForComparisonOperations3",
                .code = "1 != 2 != 3 ",
                .errors = {
                    PErr{.code = E::ChainingNotAllowedForComparisonOperations, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 10}
                },
                .verifier = IsBinary(
                    TokenType::NotEquals,
                    IsBinary(TokenType::NotEquals, IsNumber("1"), IsNumber("2")),
                    IsNumber("3")
                )
            });

            reg({
                .name = "ChainingNotAllowedForComparisonOperations4",
                .code = "x == y == z",
                .errors = {
                    PErr{.code = E::ChainingNotAllowedForComparisonOperations, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 10}
                },
                .verifier = IsBinary(
                    TokenType::Equals,
                    IsBinary(TokenType::Equals, IsIdentifier("x"), IsIdentifier("y")),
                    IsIdentifier("z")
                )
            });

            reg({
                .name = "ChainingNotAllowedForComparisonOperationsMixedOperators1",
                .code = "1 < 2 > 3",
                .errors = {
                    PErr{.code = E::ChainingNotAllowedForComparisonOperations, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                },
                .verifier = IsBinary(
                    TokenType::Greater,
                    IsBinary(TokenType::Less, IsNumber("1"), IsNumber("2")),
                    IsNumber("3")
                )
            });

            reg({
                .name = "ChainingNotAllowedForComparisonOperationsMixedOperators2",
                .code = "1 == 2 != 3",
                .errors = {
                    PErr{.code = E::ChainingNotAllowedForComparisonOperations, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 10}
                },
                .verifier = IsBinary(
                    TokenType::NotEquals,
                    IsBinary(TokenType::Equals, IsNumber("1"), IsNumber("2")),
                    IsNumber("3")
                )
            });

            return true;
        }();
    }
}
