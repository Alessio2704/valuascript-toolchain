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
                    PErr{.code = E::InvalidExpression, .line_start = 0, .column_start = 0, .line_end = 0, .column_end = 0, .skip_span_check = true}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight1",
                .code = "1 + * 2",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight2",
                .code = "1 + *",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNull())
            });

            reg({
                .name = "BinaryInvalidRight3",
                .code = "1 + - * 2",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
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
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryInvalidRight2",
                .code = "+ * 2",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryInvalidRight3",
                .code = "- + * 2",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
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
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}
                },
                .verifier = IsUnary(TokenType::Plus, IsNull())
            });

            reg({
                .name = "UnaryMissingRight1",
                .code = "- ",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 0, .column_start = 0, .line_end = 0, .column_end = 0, .skip_span_check = true}
                },
                .verifier = IsUnary(TokenType::Minus, IsNull())
            });

            reg({
                .name = "UnaryMissingRight2",
                .code = "not ",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 0, .column_start = 0, .line_end = 0, .column_end = 0, .skip_span_check = true}
                },
                .verifier = IsUnary(TokenType::Not, IsNull())
            });

            reg({
                .name = "RightAssociativeMissingOperand",
                .code = "2 ^ ^ 3 ",
                .errors = {
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                },
                .verifier = IsBinary(TokenType::Caret, IsNumber("2"), IsNull())
            });

            reg({
                .name = "ModifierInsideExpressionContext",
                .code = "1 + @test 2",
                .errors = {
                    PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 10},
                    PErr{.code = E::ModifiersAttachedToInvalidDeclaration, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 10}
                },
                .verifier = IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))
            });

            return true;
        }();
    }
}
