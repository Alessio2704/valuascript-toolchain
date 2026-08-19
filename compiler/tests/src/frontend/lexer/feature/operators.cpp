#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "token/operator_lookup.h"
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerOperatorTest : public LexerTestBase
    {
    };

    TEST_F(LexerOperatorTest, AllBinaryOperatorsDynamic)
    {
        for (const auto& op : valuascript::shared::BINARY_OPERATORS)
        {
            ExpectTokens(std::string(op.text), {
                {
                    .type = op.type,
                    .lexeme = std::string(op.text),
                    .line = 1,
                    .column = 1,
                    .start_offset = 0,
                    .length = op.text.length()
                }
            });
        }
    }

    TEST_F(LexerOperatorTest, BinaryExpressionsDynamic)
    {
        for (const auto& op : valuascript::shared::BINARY_OPERATORS)
        {
            std::string source = "a " + std::string(op.text) + " b";
            ExpectTokens(source, {
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.type = op.type, .lexeme = std::string(op.text), .line = 1, .column = 3, .start_offset = 2, .length = op.text.length()},
                {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 4 + op.text.length(), .start_offset = 3 + op.text.length(), .length = 1}
            });
        }
    }

    TEST_F(LexerOperatorTest, AllUnaryOperatorsDynamic)
    {
        for (const auto& op : valuascript::shared::UNARY_OPERATORS)
        {
            std::string source = std::string(op.text) + " a";
            ExpectTokens(source, {
                {
                    .type = op.type,
                    .lexeme = std::string(op.text),
                    .line = 1,
                    .column = 1,
                    .start_offset = 0,
                    .length = op.text.length()
                },
                {
                    .type = TokenType::Identifier,
                    .lexeme = "a",
                    .line = 1,
                    .column = op.text.length() + 2,
                    .start_offset = op.text.length() + 1,
                    .length = 1
                }
            });
        }
    }

    TEST_F(LexerOperatorTest, AssignmentAndArrow)
    {
        ExpectTokens("=", {
            {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 1, .start_offset = 0, .length = 1}
        });

        ExpectTokens("->", {
            {.type = TokenType::Arrow, .lexeme = "->", .line = 1, .column = 1, .start_offset = 0, .length = 2}
        });
    }

    TEST_F(LexerOperatorTest, MaxMunchAndConsecutiveOperators)
    {
        ExpectTokens("===", {
            {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 1, .start_offset = 0, .length = 2},
            {.type = TokenType::Assign, .lexeme = "=",  .line = 1, .column = 3, .start_offset = 2, .length = 1}
        });

        ExpectTokens("====", {
            {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 1, .start_offset = 0, .length = 2},
            {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 3, .start_offset = 2, .length = 2}
        });

        ExpectTokens("!=!=", {
            {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 1, .start_offset = 0, .length = 2},
            {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 3, .start_offset = 2, .length = 2}
        });

        ExpectTokens("!=!==", {
            {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 1, .start_offset = 0, .length = 2},
            {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 3, .start_offset = 2, .length = 2},
            {.type = TokenType::Assign,    .lexeme = "=",  .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });
    }

    TEST_F(LexerOperatorTest, OperatorsWithoutSpacesInExpressions)
    {
        ExpectTokens("a+b*c-d/e^f", {
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 1,  .start_offset = 0,  .length = 1},
            {.type = TokenType::Plus,       .lexeme = "+", .line = 1, .column = 2,  .start_offset = 1,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 3,  .start_offset = 2,  .length = 1},
            {.type = TokenType::Star,       .lexeme = "*", .line = 1, .column = 4,  .start_offset = 3,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "c", .line = 1, .column = 5,  .start_offset = 4,  .length = 1},
            {.type = TokenType::Minus,      .lexeme = "-", .line = 1, .column = 6,  .start_offset = 5,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "d", .line = 1, .column = 7,  .start_offset = 6,  .length = 1},
            {.type = TokenType::Slash,      .lexeme = "/", .line = 1, .column = 8,  .start_offset = 7,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "e", .line = 1, .column = 9,  .start_offset = 8,  .length = 1},
            {.type = TokenType::Caret,      .lexeme = "^", .line = 1, .column = 10, .start_offset = 9,  .length = 1},
            {.type = TokenType::Identifier, .lexeme = "f", .line = 1, .column = 11, .start_offset = 10, .length = 1}
        });

        ExpectTokens("a==-1", {
            {.type = TokenType::Identifier, .lexeme = "a",  .line = 1, .column = 1, .start_offset = 0, .length = 1},
            {.type = TokenType::Equals,     .lexeme = "==", .line = 1, .column = 2, .start_offset = 1, .length = 2},
            {.type = TokenType::Minus,      .lexeme = "-",  .line = 1, .column = 4, .start_offset = 3, .length = 1},
            {.type = TokenType::Number,     .lexeme = "1",  .line = 1, .column = 5, .start_offset = 4, .length = 1}
        });
    }
}
