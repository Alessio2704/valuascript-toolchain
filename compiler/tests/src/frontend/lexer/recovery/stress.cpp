#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    class LexerRecoveryStressTest : public LexerTestBase
    {
    };

    TEST_F(LexerRecoveryStressTest, MixedTokenErrors)
    {
        ExpectLexerRecovery("w = 12.\nx = .5\ny = \"unclosed string spanning to EOF",
            {
                {.code = E::UnterminatedDecimal, .line = 1, .column = 5, .start_offset = 4,  .length = 3},
                {.code = E::UnclosedString,       .line = 3, .column = 5, .start_offset = 19, .length = 32}
            },
            {
                {.type = TokenType::Identifier, .lexeme = "w",                               .line = 1, .column = 1, .start_offset = 0,  .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=",                               .line = 1, .column = 3, .start_offset = 2,  .length = 1},
                {.type = TokenType::Number,     .lexeme = "12.",                             .line = 1, .column = 5, .start_offset = 4,  .length = 3},
                {.type = TokenType::Identifier, .lexeme = "x",                               .line = 2, .column = 1, .start_offset = 8,  .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=",                               .line = 2, .column = 3, .start_offset = 10, .length = 1},
                {.type = TokenType::Number,     .lexeme = ".5",                              .line = 2, .column = 5, .start_offset = 12, .length = 2},
                {.type = TokenType::Identifier, .lexeme = "y",                               .line = 3, .column = 1, .start_offset = 15, .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=",                               .line = 3, .column = 3, .start_offset = 17, .length = 1},
                {.type = TokenType::String,     .lexeme = "\"unclosed string spanning to EOF", .line = 3, .column = 5, .start_offset = 19, .length = 32}
            });
    }

    TEST_F(LexerRecoveryStressTest, HiddenLexicalErrorsInMath)
    {
        ExpectLexerRecovery("10 + .5 * 100_ - \"unclosed",
            {
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 11, .start_offset = 10, .length = 4},
                {.code = E::UnclosedString,                   .line = 1, .column = 18, .start_offset = 17, .length = 9}
            },
            {
                {.type = TokenType::Number,     .lexeme = "10",        .line = 1, .column = 1,  .start_offset = 0,  .length = 2},
                {.type = TokenType::Plus,       .lexeme = "+",         .line = 1, .column = 4,  .start_offset = 3,  .length = 1},
                {.type = TokenType::Number,     .lexeme = ".5",        .line = 1, .column = 6,  .start_offset = 5,  .length = 2},
                {.type = TokenType::Star,       .lexeme = "*",         .line = 1, .column = 9,  .start_offset = 8,  .length = 1},
                {.type = TokenType::Number,     .lexeme = "100_",      .line = 1, .column = 11, .start_offset = 10, .length = 4},
                {.type = TokenType::Minus,      .lexeme = "-",         .line = 1, .column = 16, .start_offset = 15, .length = 1},
                {.type = TokenType::String,     .lexeme = "\"unclosed", .line = 1, .column = 18, .start_offset = 17, .length = 9}
            });
    }

    TEST_F(LexerRecoveryStressTest, RapidFireCorruptions)
    {
        std::string source = "\"unclosed 1\n12.\n\"unclosed 2\n.9\n1_a";

        ExpectLexerRecovery(source,
            {
                {.code = E::UnclosedString,                   .line = 1, .column = 1,  .start_offset = 0,  .length = 11},
                {.code = E::UnterminatedDecimal,              .line = 2, .column = 1,  .start_offset = 12, .length = 3},
                {.code = E::UnclosedString,                   .line = 3, .column = 1,  .start_offset = 16, .length = 11},
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 5, .column = 1,  .start_offset = 31, .length = 2}
            },
            {
                {.type = TokenType::String,     .lexeme = "\"unclosed 1", .line = 1, .column = 1, .start_offset = 0,  .length = 11},
                {.type = TokenType::Number,     .lexeme = "12.",          .line = 2, .column = 1, .start_offset = 12, .length = 3},
                {.type = TokenType::String,     .lexeme = "\"unclosed 2", .line = 3, .column = 1, .start_offset = 16, .length = 11},
                {.type = TokenType::Number,     .lexeme = ".9",           .line = 4, .column = 1, .start_offset = 28, .length = 2},
                {.type = TokenType::Number,     .lexeme = "1_",           .line = 5, .column = 1, .start_offset = 31, .length = 2},
                {.type = TokenType::Identifier, .lexeme = "a",            .line = 5, .column = 3, .start_offset = 33, .length = 1}
            });
    }

    TEST_F(LexerRecoveryStressTest, InterleavedValidAndInvalid)
    {
        std::string source =
            "valid1 = 100\n"
            "bad1 = $\n"
            "valid2 = \"test\"\n"
            "bad2 = ~\n"
            "valid3 = .99\n"
            "valid4 = 0.99";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter, .line = 2, .column = 8, .start_offset = 20, .length = 1},
                {.code = E::InvalidCharacter, .line = 4, .column = 8, .start_offset = 45, .length = 1}
            },
            {
                {.type = TokenType::Identifier, .lexeme = "valid1", .line = 1, .column = 1,  .start_offset = 0,  .length = 6},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 1, .column = 8,  .start_offset = 7,  .length = 1},
                {.type = TokenType::Number,     .lexeme = "100",    .line = 1, .column = 10, .start_offset = 9,  .length = 3},
                {.type = TokenType::Identifier, .lexeme = "bad1",   .line = 2, .column = 1,  .start_offset = 13, .length = 4},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 2, .column = 6,  .start_offset = 18, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "valid2", .line = 3, .column = 1,  .start_offset = 22, .length = 6},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 3, .column = 8,  .start_offset = 29, .length = 1},
                {.type = TokenType::String,     .lexeme = "\"test\"", .line = 3, .column = 10, .start_offset = 31, .length = 6},
                {.type = TokenType::Identifier, .lexeme = "bad2",   .line = 4, .column = 1,  .start_offset = 38, .length = 4},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 4, .column = 6,  .start_offset = 43, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "valid3", .line = 5, .column = 1,  .start_offset = 47, .length = 6},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 5, .column = 8,  .start_offset = 54, .length = 1},
                {.type = TokenType::Number,     .lexeme = ".99",    .line = 5, .column = 10, .start_offset = 56, .length = 3},
                {.type = TokenType::Identifier, .lexeme = "valid4", .line = 6, .column = 1,  .start_offset = 60, .length = 6},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 6, .column = 8,  .start_offset = 67, .length = 1},
                {.type = TokenType::Number,     .lexeme = "0.99",   .line = 6, .column = 10, .start_offset = 69, .length = 4}
            });
    }

    TEST_F(LexerRecoveryStressTest, WhitespaceAndIndentationTracking)
    {
        std::string source = "\n\n\t\tspaced =   $\n    padded = .123\n    carriage = 12.\n";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter,   .line = 3, .column = 14, .start_offset = 15, .length = 1},
                {.code = E::UnterminatedDecimal, .line = 5, .column = 16, .start_offset = 50, .length = 3}
            },
            {
                {.type = TokenType::Identifier, .lexeme = "spaced",   .line = 3, .column = 3,  .start_offset = 4,  .length = 6},
                {.type = TokenType::Assign,     .lexeme = "=",        .line = 3, .column = 10, .start_offset = 11, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "padded",   .line = 4, .column = 5,  .start_offset = 21, .length = 6},
                {.type = TokenType::Assign,     .lexeme = "=",        .line = 4, .column = 12, .start_offset = 28, .length = 1},
                {.type = TokenType::Number,     .lexeme = ".123",     .line = 4, .column = 14, .start_offset = 30, .length = 4},
                {.type = TokenType::Identifier, .lexeme = "carriage", .line = 5, .column = 5,  .start_offset = 39, .length = 8},
                {.type = TokenType::Assign,     .lexeme = "=",        .line = 5, .column = 14, .start_offset = 48, .length = 1},
                {.type = TokenType::Number,     .lexeme = "12.",      .line = 5, .column = 16, .start_offset = 50, .length = 3}
            });
    }

    TEST_F(LexerRecoveryStressTest, LargeScaleStressTest)
    {
        std::string source =
            "\n\n"
            "value = 100 \n"
            "invalid1 = $  \n"
            "partial = 0.\n"
            "invalid2 = ~\n\n"
            "unclosed = \"started\n";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter,   .line = 4, .column = 12, .start_offset = 26, .length = 1},
                {.code = E::UnterminatedDecimal, .line = 5, .column = 11, .start_offset = 40, .length = 2},
                {.code = E::InvalidCharacter,   .line = 6, .column = 12, .start_offset = 54, .length = 1},
                {.code = E::UnclosedString,     .line = 8, .column = 12, .start_offset = 68, .length = 8}
            },
            {
                {.type = TokenType::Identifier, .lexeme = "value",     .line = 3, .column = 1,  .start_offset = 2,  .length = 5},
                {.type = TokenType::Assign,     .lexeme = "=",         .line = 3, .column = 7,  .start_offset = 8,  .length = 1},
                {.type = TokenType::Number,     .lexeme = "100",       .line = 3, .column = 9,  .start_offset = 10, .length = 3},
                {.type = TokenType::Identifier, .lexeme = "invalid1",  .line = 4, .column = 1,  .start_offset = 15, .length = 8},
                {.type = TokenType::Assign,     .lexeme = "=",         .line = 4, .column = 10, .start_offset = 24, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "partial",   .line = 5, .column = 1,  .start_offset = 30, .length = 7},
                {.type = TokenType::Assign,     .lexeme = "=",         .line = 5, .column = 9,  .start_offset = 38, .length = 1},
                {.type = TokenType::Number,     .lexeme = "0.",        .line = 5, .column = 11, .start_offset = 40, .length = 2},
                {.type = TokenType::Identifier, .lexeme = "invalid2",  .line = 6, .column = 1,  .start_offset = 43, .length = 8},
                {.type = TokenType::Assign,     .lexeme = "=",         .line = 6, .column = 10, .start_offset = 52, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "unclosed",  .line = 8, .column = 1,  .start_offset = 57, .length = 8},
                {.type = TokenType::Assign,     .lexeme = "=",         .line = 8, .column = 10, .start_offset = 66, .length = 1},
                {.type = TokenType::String,     .lexeme = "\"started", .line = 8, .column = 12, .start_offset = 68, .length = 8}
            });
    }
}
