#include <gtest/gtest.h>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerDelimiterTest : public LexerTestBase
    {
    };

    TEST_F(LexerDelimiterTest, EmptySourceAndWhitespaceOnly)
    {
        ExpectTokens("");
        ExpectTokens("   ");
        ExpectTokens("\t\t");
        ExpectTokens("\n\n\n");
        ExpectTokens("  \n \t \n  ");
    }

    TEST_F(LexerDelimiterTest, ParenthesesBracketsAndBraces)
    {
        ExpectTokens("( ) [ ] { }", {
            {.type = TokenType::LeftParen,    .lexeme = "(", .line = 1, .column = 1,  .start_offset = 0,  .length = 1},
            {.type = TokenType::RightParen,   .lexeme = ")", .line = 1, .column = 3,  .start_offset = 2,  .length = 1},
            {.type = TokenType::LeftBracket,  .lexeme = "[", .line = 1, .column = 5,  .start_offset = 4,  .length = 1},
            {.type = TokenType::RightBracket, .lexeme = "]", .line = 1, .column = 7,  .start_offset = 6,  .length = 1},
            {.type = TokenType::LeftBrace,    .lexeme = "{", .line = 1, .column = 9,  .start_offset = 8,  .length = 1},
            {.type = TokenType::RightBrace,   .lexeme = "}", .line = 1, .column = 11, .start_offset = 10, .length = 1}
        });
    }

    TEST_F(LexerDelimiterTest, PunctuationTokens)
    {
        ExpectTokens(", : .", {
            {.type = TokenType::Comma,     .lexeme = ",", .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::Colon,     .lexeme = ":", .line = 1, .column = 3, .start_offset = 2, .length = 1},
            {.type = TokenType::Dot,       .lexeme = ".", .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });
    }

    TEST_F(LexerDelimiterTest, HashAndAtSymbols)
    {
        ExpectTokens("# @", {
            {.type = TokenType::Hash,      .lexeme = "#", .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::At,        .lexeme = "@", .line = 1, .column = 3, .start_offset = 2, .length = 1}
        });
    }

    TEST_F(LexerDelimiterTest, CompactPunctuationSequences)
    {
        ExpectTokens("(a,b)", {
            {.type = TokenType::LeftParen,  .lexeme = "(", .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 2, .start_offset = 1, .length = 1},
            {.type = TokenType::Comma,      .lexeme = ",", .line = 1, .column = 3, .start_offset = 2, .length = 1},
            {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 4, .start_offset = 3, .length = 1},
            {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });

        ExpectTokens("[1,2]", {
            {.type = TokenType::LeftBracket,  .lexeme = "[", .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::Number,       .lexeme = "1", .line = 1, .column = 2, .start_offset = 1, .length = 1},
            {.type = TokenType::Comma,        .lexeme = ",", .line = 1, .column = 3, .start_offset = 2, .length = 1},
            {.type = TokenType::Number,       .lexeme = "2", .line = 1, .column = 4, .start_offset = 3, .length = 1},
            {.type = TokenType::RightBracket, .lexeme = "]", .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });

        ExpectTokens("{k:v}", {
            {.type = TokenType::LeftBrace,  .lexeme = "{", .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::Identifier, .lexeme = "k", .line = 1, .column = 2, .start_offset = 1, .length = 1},
            {.type = TokenType::Colon,      .lexeme = ":", .line = 1, .column = 3, .start_offset = 2, .length = 1},
            {.type = TokenType::Identifier, .lexeme = "v", .line = 1, .column = 4, .start_offset = 3, .length = 1},
            {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });

        ExpectTokens("a.b.c", {
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::Dot,        .lexeme = ".", .line = 1, .column = 2, .start_offset = 1, .length = 1},
            {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 3, .start_offset = 2, .length = 1},
            {.type = TokenType::Dot,        .lexeme = ".", .line = 1, .column = 4, .start_offset = 3, .length = 1},
            {.type = TokenType::Identifier, .lexeme = "c", .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });
    }

    TEST_F(LexerDelimiterTest, DirectiveAndAttributePrefixes)
    {
        ExpectTokens("#mode @inline", {
            {.type = TokenType::Hash,       .lexeme = "#",       .line = 1, .column = 1,  .start_offset = 0,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "mode",    .line = 1, .column = 2,  .start_offset = 1,  .length = 4},
            {.type = TokenType::At,         .lexeme = "@",       .line = 1, .column = 7,  .start_offset = 6,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "inline",  .line = 1, .column = 8,  .start_offset = 7,  .length = 6}
        });
    }
}
