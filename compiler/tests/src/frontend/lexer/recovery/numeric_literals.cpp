#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    class LexerRecoveryNumericLiteralsTest : public LexerTestBase
    {
    };

    TEST_F(LexerRecoveryNumericLiteralsTest, UnterminatedDecimalIsolated)
    {
        ExpectLexerRecovery("1.",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 1, .start_offset = 0, .length = 2}},
            {
                {.type = TokenType::Number, .lexeme = "1.", .line = 1, .column = 1, .start_offset = 0, .length = 2}
            });

        ExpectLexerRecovery("1_230.",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 1, .start_offset = 0, .length = 6}},
            {
                {.type = TokenType::Number, .lexeme = "1_230.", .line = 1, .column = 1, .start_offset = 0, .length = 6}
            });
    }

    TEST_F(LexerRecoveryNumericLiteralsTest, UnterminatedDecimalInExpression)
    {
        ExpectLexerRecovery("1. + 2",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 1, .start_offset = 0, .length = 2}},
            {
                {.type = TokenType::Number, .lexeme = "1.", .line = 1, .column = 1, .start_offset = 0, .length = 2},
                {.type = TokenType::Plus,   .lexeme = "+",  .line = 1, .column = 4, .start_offset = 3, .length = 1},
                {.type = TokenType::Number, .lexeme = "2",  .line = 1, .column = 6, .start_offset = 5, .length = 1}
            });

        ExpectLexerRecovery("a = 1.",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 5, .start_offset = 4, .length = 2}},
            {
                {.type = TokenType::Identifier, .lexeme = "a",  .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=",  .line = 1, .column = 3, .start_offset = 2, .length = 1},
                {.type = TokenType::Number,     .lexeme = "1.", .line = 1, .column = 5, .start_offset = 4, .length = 2}
            });
    }

    TEST_F(LexerRecoveryNumericLiteralsTest, TrailingSeparatorIsolated)
    {
        ExpectLexerRecovery("1_",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 1, .start_offset = 0, .length = 2}},
            {
                {.type = TokenType::Number, .lexeme = "1_", .line = 1, .column = 1, .start_offset = 0, .length = 2}
            });

        ExpectLexerRecovery("100_",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 1, .start_offset = 0, .length = 4}},
            {
                {.type = TokenType::Number, .lexeme = "100_", .line = 1, .column = 1, .start_offset = 0, .length = 4}
            });
    }

    TEST_F(LexerRecoveryNumericLiteralsTest, TrailingSeparatorInExpression)
    {
        ExpectLexerRecovery("1_ * 2",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 1, .start_offset = 0, .length = 2}},
            {
                {.type = TokenType::Number, .lexeme = "1_", .line = 1, .column = 1, .start_offset = 0, .length = 2},
                {.type = TokenType::Star,   .lexeme = "*",  .line = 1, .column = 4, .start_offset = 3, .length = 1},
                {.type = TokenType::Number, .lexeme = "2",  .line = 1, .column = 6, .start_offset = 5, .length = 1}
            });
    }

    TEST_F(LexerRecoveryNumericLiteralsTest, DoubleAndMalformedSeparators)
    {
        ExpectLexerRecovery("1__000",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 1, .start_offset = 0, .length = 2}},
            {
                {.type = TokenType::Number,     .lexeme = "1_",   .line = 1, .column = 1, .start_offset = 0, .length = 2},
                {.type = TokenType::Identifier, .lexeme = "_000", .line = 1, .column = 3, .start_offset = 2, .length = 4}
            });

        ExpectLexerRecovery("12__3",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 1, .start_offset = 0, .length = 3}},
            {
                {.type = TokenType::Number,     .lexeme = "12_", .line = 1, .column = 1, .start_offset = 0, .length = 3},
                {.type = TokenType::Identifier, .lexeme = "_3",  .line = 1, .column = 4, .start_offset = 3, .length = 2}
            });

        ExpectLexerRecovery("45._",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 1, .start_offset = 0, .length = 3}},
            {
                {.type = TokenType::Number,     .lexeme = "45.", .line = 1, .column = 1, .start_offset = 0, .length = 3},
                {.type = TokenType::Identifier, .lexeme = "_",   .line = 1, .column = 4, .start_offset = 3, .length = 1}
            });
    }

    TEST_F(LexerRecoveryNumericLiteralsTest, MultipleNumericErrorsAcrossLines)
    {
        ExpectLexerRecovery("12__3\n45._\n100_",
            {
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 1,  .start_offset = 0,  .length = 3},
                {.code = E::UnterminatedDecimal,              .line = 2, .column = 1,  .start_offset = 6,  .length = 3},
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 3, .column = 1,  .start_offset = 11, .length = 4}
            },
            {
                {.type = TokenType::Number,     .lexeme = "12_",  .line = 1, .column = 1, .start_offset = 0,  .length = 3},
                {.type = TokenType::Identifier, .lexeme = "_3",   .line = 1, .column = 4, .start_offset = 3,  .length = 2},
                {.type = TokenType::Number,     .lexeme = "45.",  .line = 2, .column = 1, .start_offset = 6,  .length = 3},
                {.type = TokenType::Identifier, .lexeme = "_",    .line = 2, .column = 4, .start_offset = 9,  .length = 1},
                {.type = TokenType::Number,     .lexeme = "100_", .line = 3, .column = 1, .start_offset = 11, .length = 4}
            });
    }
}
