#include <gtest/gtest.h>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerIdentifierTest : public LexerTestBase
    {
    };

    TEST_F(LexerIdentifierTest, SingleCharacterIdentifiers)
    {
        ExpectTokens("a", {
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 1, .start_offset = 0, .length = 1}
        });

        ExpectTokens("z", {
            {.type = TokenType::Identifier, .lexeme = "z", .line = 1, .column = 1, .start_offset = 0, .length = 1}
        });

        ExpectTokens("X", {
            {.type = TokenType::Identifier, .lexeme = "X", .line = 1, .column = 1, .start_offset = 0, .length = 1}
        });

        ExpectTokens("_", {
            {.type = TokenType::Identifier, .lexeme = "_", .line = 1, .column = 1, .start_offset = 0, .length = 1}
        });
    }

    TEST_F(LexerIdentifierTest, MultiCharacterIdentifiers)
    {
        ExpectTokens("foo", {
            {.type = TokenType::Identifier, .lexeme = "foo", .line = 1, .column = 1, .start_offset = 0, .length = 3}
        });

        ExpectTokens("myVariable", {
            {.type = TokenType::Identifier, .lexeme = "myVariable", .line = 1, .column = 1, .start_offset = 0, .length = 10}
        });

        ExpectTokens("UserAccountManager", {
            {.type = TokenType::Identifier, .lexeme = "UserAccountManager", .line = 1, .column = 1, .start_offset = 0, .length = 18}
        });

        ExpectTokens("user_id_123", {
            {.type = TokenType::Identifier, .lexeme = "user_id_123", .line = 1, .column = 1, .start_offset = 0, .length = 11}
        });
    }

    TEST_F(LexerIdentifierTest, UnderscorePrefixedAndSuffixed)
    {
        ExpectTokens("_hidden", {
            {.type = TokenType::Identifier, .lexeme = "_hidden", .line = 1, .column = 1, .start_offset = 0, .length = 7}
        });

        ExpectTokens("__init__", {
            {.type = TokenType::Identifier, .lexeme = "__init__", .line = 1, .column = 1, .start_offset = 0, .length = 8}
        });

        ExpectTokens("value_", {
            {.type = TokenType::Identifier, .lexeme = "value_", .line = 1, .column = 1, .start_offset = 0, .length = 6}
        });

        ExpectTokens("_123", {
            {.type = TokenType::Identifier, .lexeme = "_123", .line = 1, .column = 1, .start_offset = 0, .length = 4}
        });
    }

    TEST_F(LexerIdentifierTest, MultipleIdentifiersSeparatedByWhitespace)
    {
        ExpectTokens("foo bar baz", {
            {.type = TokenType::Identifier, .lexeme = "foo", .line = 1, .column = 1, .start_offset = 0, .length = 3},
            {.type = TokenType::Identifier, .lexeme = "bar", .line = 1, .column = 5, .start_offset = 4, .length = 3},
            {.type = TokenType::Identifier, .lexeme = "baz", .line = 1, .column = 9, .start_offset = 8, .length = 3}
        });
    }
}
