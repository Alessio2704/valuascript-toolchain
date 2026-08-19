#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    class LexerRecoveryInvalidCharactersTest : public LexerTestBase
    {
    };

    TEST_F(LexerRecoveryInvalidCharactersTest, IsolatedInvalidCharacters)
    {
        ExpectLexerRecovery("$",
            {{.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1}},
            {});

        ExpectLexerRecovery("&",
            {{.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1}},
            {});

        ExpectLexerRecovery("~",
            {{.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1}},
            {});

        ExpectLexerRecovery("\\",
            {{.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1}},
            {});

        ExpectLexerRecovery("?",
            {{.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1}},
            {});
    }

    TEST_F(LexerRecoveryInvalidCharactersTest, StrayPercentagePrefix)
    {
        ExpectLexerRecovery("%1",
            {{.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1}},
            {
                {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 2, .start_offset = 1, .length = 1}
            });
    }

    TEST_F(LexerRecoveryInvalidCharactersTest, InvalidCharactersInTokenStream)
    {
        ExpectLexerRecovery("a $ b",
            {{.code = E::InvalidCharacter, .line = 1, .column = 3, .start_offset = 2, .length = 1}},
            {
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 5, .start_offset = 4, .length = 1}
            });

        ExpectLexerRecovery("x + & y",
            {{.code = E::InvalidCharacter, .line = 1, .column = 5, .start_offset = 4, .length = 1}},
            {
                {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.type = TokenType::Plus,       .lexeme = "+", .line = 1, .column = 3, .start_offset = 2, .length = 1},
                {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 7, .start_offset = 6, .length = 1}
            });
    }

    TEST_F(LexerRecoveryInvalidCharactersTest, MultipleInvalidCharactersAcrossLines)
    {
        ExpectLexerRecovery("a = $\nb = ~\nc = \\",
            {
                {.code = E::InvalidCharacter, .line = 1, .column = 5, .start_offset = 4,  .length = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 5, .start_offset = 10, .length = 1},
                {.code = E::InvalidCharacter, .line = 3, .column = 5, .start_offset = 16, .length = 1}
            },
            {
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 1, .start_offset = 0,  .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=", .line = 1, .column = 3, .start_offset = 2,  .length = 1},
                {.type = TokenType::Identifier, .lexeme = "b", .line = 2, .column = 1, .start_offset = 6,  .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=", .line = 2, .column = 3, .start_offset = 8,  .length = 1},
                {.type = TokenType::Identifier, .lexeme = "c", .line = 3, .column = 1, .start_offset = 12, .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=", .line = 3, .column = 3, .start_offset = 14, .length = 1}
            });
    }

    TEST_F(LexerRecoveryInvalidCharactersTest, SuccessiveInvalidCharacters)
    {
        ExpectLexerRecovery("$$$\n~~~",
            {
                {.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.code = E::InvalidCharacter, .line = 1, .column = 2, .start_offset = 1, .length = 1},
                {.code = E::InvalidCharacter, .line = 1, .column = 3, .start_offset = 2, .length = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 1, .start_offset = 4, .length = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 2, .start_offset = 5, .length = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 3, .start_offset = 6, .length = 1}
            },
            {});
    }

    TEST_F(LexerRecoveryInvalidCharactersTest, PureGarbageFile)
    {
        ExpectLexerRecovery("?\\`\n~|\n$$$",
            {
                {.code = E::InvalidCharacter, .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.code = E::InvalidCharacter, .line = 1, .column = 2, .start_offset = 1, .length = 1},
                {.code = E::InvalidCharacter, .line = 1, .column = 3, .start_offset = 2, .length = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 1, .start_offset = 4, .length = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 2, .start_offset = 5, .length = 1},
                {.code = E::InvalidCharacter, .line = 3, .column = 1, .start_offset = 7, .length = 1},
                {.code = E::InvalidCharacter, .line = 3, .column = 2, .start_offset = 8, .length = 1},
                {.code = E::InvalidCharacter, .line = 3, .column = 3, .start_offset = 9, .length = 1}
            },
            {});
    }
}
