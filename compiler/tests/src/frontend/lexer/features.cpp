#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerHappyPathTest : public LexerTestBase
    {
    };

    TEST_F(LexerHappyPathTest, Number)
    {
        ExpectTokens("let a = 1000", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Number, .lexeme = "1000", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, Number1)
    {
        ExpectTokens("1000", {
                         {.type = TokenType::Number, .lexeme = "1000", .line = 1, .column = 1},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 5}
                     });
    }

    TEST_F(LexerHappyPathTest, Number2)
    {
        ExpectTokens("1000.5", {
                         {.type = TokenType::Number, .lexeme = "1000.5", .line = 1, .column = 1},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 7}
                     });
    }

    TEST_F(LexerHappyPathTest, Number3)
    {
        ExpectTokens("1_000.5", {
                         {.type = TokenType::Number, .lexeme = "1_000.5", .line = 1, .column = 1},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 8}
                     });
    }

    TEST_F(LexerHappyPathTest, Number4)
    {
        ExpectTokens("0.5", {
                         {.type = TokenType::Number, .lexeme = "0.5", .line = 1, .column = 1},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 4}
                     });
    }

    TEST_F(LexerHappyPathTest, NumberPercentage1)
    {
        ExpectTokens("let a = 5%", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::PercentageLiteral, .lexeme = "5%", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 11}
                     });
    }

    TEST_F(LexerHappyPathTest, NumberPercentage2)
    {
        ExpectTokens("let a = 0.5%", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::PercentageLiteral, .lexeme = "0.5%", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, NumberPercentage3)
    {
        ExpectTokens("let a = 1_000.5%", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::PercentageLiteral, .lexeme = "1_000.5%", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 17}
                     });
    }

    TEST_F(LexerHappyPathTest, String)
    {
        ExpectTokens("let a = \"string\"", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::String, .lexeme = "\"string\"", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 17}
                     });
    }

    TEST_F(LexerHappyPathTest, VectorLiteral)
    {
        ExpectTokens("let a = [1,2,3]", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftBracket, .lexeme = "[", .line = 1, .column = 9},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 10},
                         {.type = TokenType::Comma, .lexeme = ",", .line = 1, .column = 11},
                         {.type = TokenType::Number, .lexeme = "2", .line = 1, .column = 12},
                         {.type = TokenType::Comma, .lexeme = ",", .line = 1, .column = 13},
                         {.type = TokenType::Number, .lexeme = "3", .line = 1, .column = 14},
                         {.type = TokenType::RightBracket, .lexeme = "]", .line = 1, .column = 15},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 16}
                     });
    }

    TEST_F(LexerHappyPathTest, Boolean)
    {
        ExpectTokens("let a = true", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::True, .lexeme = "true", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, Cname1)
    {
        ExpectTokens("let _a = false", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "_a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 8},
                         {.type = TokenType::False, .lexeme = "false", .line = 1, .column = 10},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, IdentifierContainingKeyword)
    {
        ExpectTokens("let ifthenelse = 1", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "ifthenelse", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 16},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 18},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 19}
                     });
    }

    TEST_F(LexerHappyPathTest, FunctionCall)
    {
        ExpectTokens("let a = some_func()", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "some_func", .line = 1, .column = 9},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 18},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 19},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 20}
                     });
    }

    TEST_F(LexerHappyPathTest, MultiAssignment)
    {
        ExpectTokens("let a, b = some_func()", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Comma, .lexeme = ",", .line = 1, .column = 6},
                         {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 8},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 10},
                         {.type = TokenType::Identifier, .lexeme = "some_func", .line = 1, .column = 12},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 21},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 22},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 23}
                     });
    }

    TEST_F(LexerHappyPathTest, ConditionalExpression)
    {
        ExpectTokens("let a = if true then 10 else 4", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::If, .lexeme = "if", .line = 1, .column = 9},
                         {.type = TokenType::True, .lexeme = "true", .line = 1, .column = 12},
                         {.type = TokenType::Then, .lexeme = "then", .line = 1, .column = 17},
                         {.type = TokenType::Number, .lexeme = "10", .line = 1, .column = 22},
                         {.type = TokenType::Else, .lexeme = "else", .line = 1, .column = 25},
                         {.type = TokenType::Number, .lexeme = "4", .line = 1, .column = 30},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 31}
                     });
    }

    TEST_F(LexerHappyPathTest, DictLiteral)
    {
        ExpectTokens("let a = {b: 1}", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 9},
                         {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 10},
                         {.type = TokenType::Colon, .lexeme = ":", .line = 1, .column = 11},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 13},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 14},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, DictLiteralSelf)
    {
        ExpectTokens("let a = {b: self.c}", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 9},
                         {.type = TokenType::Identifier, .lexeme = "b", .line = 1, .column = 10},
                         {.type = TokenType::Colon, .lexeme = ":", .line = 1, .column = 11},
                         {.type = TokenType::Self, .lexeme = "self", .line = 1, .column = 13},
                         {.type = TokenType::Dot, .lexeme = ".", .line = 1, .column = 17},
                         {.type = TokenType::Identifier, .lexeme = "c", .line = 1, .column = 18},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 19},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 20}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorOr)
    {
        ExpectTokens("let a = x or y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::Or, .lexeme = "or", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 14},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorAnd)
    {
        ExpectTokens("let a = x and y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::And, .lexeme = "and", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 15},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 16}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorNot)
    {
        ExpectTokens("let a = not x", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Not, .lexeme = "not", .line = 1, .column = 9},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 13},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 14}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorEq)
    {
        ExpectTokens("let a = x == y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 14},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorNeq)
    {
        ExpectTokens("let a = x != y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 14},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorGt)
    {
        ExpectTokens("let a = x > y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::Greater, .lexeme = ">", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 13},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 14}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorLt)
    {
        ExpectTokens("let a = x < y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::Less, .lexeme = "<", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 13},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 14}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorGte)
    {
        ExpectTokens("let a = x >= y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::GreaterEqual, .lexeme = ">=", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 14},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorLte)
    {
        ExpectTokens("let a = x <= y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::LessEqual, .lexeme = "<=", .line = 1, .column = 11},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 14},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorPow)
    {
        ExpectTokens("let a = x^y", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::Caret, .lexeme = "^", .line = 1, .column = 10},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 11},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 12}
                     });
    }


    TEST_F(LexerHappyPathTest, AccessVectorElement)
    {
        ExpectTokens("let a = x[1]", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::LeftBracket, .lexeme = "[", .line = 1, .column = 10},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 11},
                         {.type = TokenType::RightBracket, .lexeme = "]", .line = 1, .column = 12},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, DeleteVectorElement)
    {
        ExpectTokens("let a = x[:1]", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 9},
                         {.type = TokenType::LeftBracket, .lexeme = "[", .line = 1, .column = 10},
                         {.type = TokenType::Colon, .lexeme = ":", .line = 1, .column = 11},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 12},
                         {.type = TokenType::RightBracket, .lexeme = "]", .line = 1, .column = 13},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 14}
                     });
    }

    TEST_F(LexerHappyPathTest, ParenthesisInAssignment)
    {
        ExpectTokens("let a = (x + y) * z", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 9},
                         {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 10},
                         {.type = TokenType::Plus, .lexeme = "+", .line = 1, .column = 12},
                         {.type = TokenType::Identifier, .lexeme = "y", .line = 1, .column = 14},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 15},
                         {.type = TokenType::Star, .lexeme = "*", .line = 1, .column = 17},
                         {.type = TokenType::Identifier, .lexeme = "z", .line = 1, .column = 19},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 20}
                     });
    }


    TEST_F(LexerHappyPathTest, DirectiveIterations)
    {
        ExpectTokens("#iterations = 1000", {
                         {.type = TokenType::Hash, .lexeme = "#", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "iterations", .line = 1, .column = 2},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 13},
                         {.type = TokenType::Number, .lexeme = "1000", .line = 1, .column = 15},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 19}
                     });
    }

    TEST_F(LexerHappyPathTest, DirectiveOutput)
    {
        ExpectTokens("#output = my_var", {
                         {.type = TokenType::Hash, .lexeme = "#", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "output", .line = 1, .column = 2},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 9},
                         {.type = TokenType::Identifier, .lexeme = "my_var", .line = 1, .column = 11},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 17}
                     });
    }

    TEST_F(LexerHappyPathTest, DirectiveOutputFile)
    {
        ExpectTokens("#output_file = \"results.csv\"", {
                         {.type = TokenType::Hash, .lexeme = "#", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "output_file", .line = 1, .column = 2},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 14},
                         {.type = TokenType::String, .lexeme = "\"results.csv\"", .line = 1, .column = 16},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 29}
                     });
    }

    TEST_F(LexerHappyPathTest, DirectiveModule)
    {
        ExpectTokens("#module", {
                         {.type = TokenType::Hash, .lexeme = "#", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "module", .line = 1, .column = 2},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 8}
                     });
    }


    TEST_F(LexerHappyPathTest, ImportStatement)
    {
        ExpectTokens("import \"file/path\"", {
                         {.type = TokenType::Import, .lexeme = "import", .line = 1, .column = 1},
                         {.type = TokenType::String, .lexeme = "\"file/path\"", .line = 1, .column = 8},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 19}
                     });
    }


    TEST_F(LexerHappyPathTest, TypealiasStatement)
    {
        ExpectTokens("typealias Container = vector<int>", {
                         {.type = TokenType::Typealias, .lexeme = "typealias", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "Container", .line = 1, .column = 11},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 21},
                         {.type = TokenType::Identifier, .lexeme = "vector", .line = 1, .column = 23},
                         {.type = TokenType::Less, .lexeme = "<", .line = 1, .column = 29},
                         {.type = TokenType::Identifier, .lexeme = "int", .line = 1, .column = 30},
                         {.type = TokenType::Greater, .lexeme = ">", .line = 1, .column = 33},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 34}
                     });
    }


    TEST_F(LexerHappyPathTest, StructDefinition)
    {
        ExpectTokens("struct Assumption { cagr: Decimal }", {
                         {.type = TokenType::Struct, .lexeme = "struct", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "Assumption", .line = 1, .column = 8},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 19},
                         {.type = TokenType::Identifier, .lexeme = "cagr", .line = 1, .column = 21},
                         {.type = TokenType::Colon, .lexeme = ":", .line = 1, .column = 25},
                         {.type = TokenType::Identifier, .lexeme = "Decimal", .line = 1, .column = 27},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 35},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 36}
                     });
    }


    TEST_F(LexerHappyPathTest, MemberAccess1)
    {
        ExpectTokens("model.cagr", {
                         {.type = TokenType::Identifier, .lexeme = "model", .line = 1, .column = 1},
                         {.type = TokenType::Dot, .lexeme = ".", .line = 1, .column = 6},
                         {.type = TokenType::Identifier, .lexeme = "cagr", .line = 1, .column = 7},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 11}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccessUnderscoreIdentifier1)
    {
        ExpectTokens("_.prop", {
                         {.type = TokenType::Identifier, .lexeme = "_", .line = 1, .column = 1},
                         {.type = TokenType::Dot, .lexeme = ".", .line = 1, .column = 2},
                         {.type = TokenType::Identifier, .lexeme = "prop", .line = 1, .column = 3},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 7}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccessUnderscoreIdentifier2)
    {
        ExpectTokens("_0.prop", {
                         {.type = TokenType::Identifier, .lexeme = "_0", .line = 1, .column = 1},
                         {.type = TokenType::Dot, .lexeme = ".", .line = 1, .column = 3},
                         {.type = TokenType::Identifier, .lexeme = "prop", .line = 1, .column = 4},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 8}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess2)
    {
        ExpectTokens("let a = [].5", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftBracket, .lexeme = "[", .line = 1, .column = 9},
                         {.type = TokenType::RightBracket, .lexeme = "]", .line = 1, .column = 10},
                         {.type = TokenType::Number, .lexeme = ".5", .line = 1, .column = 11},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess3)
    {
        ExpectTokens("let a = ().5", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 9},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 10},
                         {.type = TokenType::Number, .lexeme = ".5", .line = 1, .column = 11},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess4)
    {
        ExpectTokens("let a = {}.5", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 9},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 10},
                         {.type = TokenType::Number, .lexeme = ".5", .line = 1, .column = 11},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 13}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess5)
    {
        ExpectTokens("let a = {a: 1}.5", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 9},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 10},
                         {.type = TokenType::Colon, .lexeme = ":", .line = 1, .column = 11},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 13},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 14},
                         {.type = TokenType::Number, .lexeme = ".5", .line = 1, .column = 15},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 17}
                     });
    }


    TEST_F(LexerHappyPathTest, AtSeparator)
    {
        ExpectTokens("let a = 1_000", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Number, .lexeme = "1_000", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 14}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloat)
    {
        ExpectTokens("let a = 1.3242", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Number, .lexeme = "1.3242", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloatAndSeparator)
    {
        ExpectTokens("let a = 1_103.3_242", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Number, .lexeme = "1_103.3_242", .line = 1, .column = 9},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 20}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloatSigned1)
    {
        ExpectTokens("let a = +1_103.3_242", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Plus, .lexeme = "+", .line = 1, .column = 9},
                         {.type = TokenType::Number, .lexeme = "1_103.3_242", .line = 1, .column = 10},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 21}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloatSigned2)
    {
        ExpectTokens("let a = -1_103.3_242", {
                         {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                         {.type = TokenType::Minus, .lexeme = "-", .line = 1, .column = 9},
                         {.type = TokenType::Number, .lexeme = "1_103.3_242", .line = 1, .column = 10},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 21}
                     });
    }


    TEST_F(LexerHappyPathTest, FuncBasic)
    {
        ExpectTokens("func test() -> scalar { return 1 }", {
                         {.type = TokenType::Func, .lexeme = "func", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "test", .line = 1, .column = 6},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 10},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 11},
                         {.type = TokenType::Arrow, .lexeme = "->", .line = 1, .column = 13},
                         {.type = TokenType::Identifier, .lexeme = "scalar", .line = 1, .column = 16},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 23},
                         {.type = TokenType::Return, .lexeme = "return", .line = 1, .column = 25},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 32},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 34},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 35}
                     });
    }

    TEST_F(LexerHappyPathTest, FuncParams)
    {
        ExpectTokens("func test(a: scalar) -> scalar { return 1 }", {
                         {.type = TokenType::Func, .lexeme = "func", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "test", .line = 1, .column = 6},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 10},
                         {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 11},
                         {.type = TokenType::Colon, .lexeme = ":", .line = 1, .column = 12},
                         {.type = TokenType::Identifier, .lexeme = "scalar", .line = 1, .column = 14},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 20},
                         {.type = TokenType::Arrow, .lexeme = "->", .line = 1, .column = 22},
                         {.type = TokenType::Identifier, .lexeme = "scalar", .line = 1, .column = 25},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 32},
                         {.type = TokenType::Return, .lexeme = "return", .line = 1, .column = 34},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 41},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 43},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 44}
                     });
    }

    TEST_F(LexerHappyPathTest, FuncTupleReturn)
    {
        ExpectTokens("func test() -> (scalar, vector) { return 1 }", {
                         {.type = TokenType::Func, .lexeme = "func", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "test", .line = 1, .column = 6},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 10},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 11},
                         {.type = TokenType::Arrow, .lexeme = "->", .line = 1, .column = 13},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 16},
                         {.type = TokenType::Identifier, .lexeme = "scalar", .line = 1, .column = 17},
                         {.type = TokenType::Comma, .lexeme = ",", .line = 1, .column = 23},
                         {.type = TokenType::Identifier, .lexeme = "vector", .line = 1, .column = 25},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 31},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 33},
                         {.type = TokenType::Return, .lexeme = "return", .line = 1, .column = 35},
                         {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 42},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 1, .column = 44},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 45}
                     });
    }

    TEST_F(LexerHappyPathTest, FuncDocstring)
    {
        ExpectTokens("func test() -> scalar { \"\"\"Docs\"\"\" \n return 1 }", {
                         {.type = TokenType::Func, .lexeme = "func", .line = 1, .column = 1},
                         {.type = TokenType::Identifier, .lexeme = "test", .line = 1, .column = 6},
                         {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 10},
                         {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 11},
                         {.type = TokenType::Arrow, .lexeme = "->", .line = 1, .column = 13},
                         {.type = TokenType::Identifier, .lexeme = "scalar", .line = 1, .column = 16},
                         {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 23},
                         {.type = TokenType::DocString, .lexeme = "\"\"\"Docs\"\"\"", .line = 1, .column = 25},
                         {.type = TokenType::Return, .lexeme = "return", .line = 2, .column = 2},
                         {.type = TokenType::Number, .lexeme = "1", .line = 2, .column = 9},
                         {.type = TokenType::RightBrace, .lexeme = "}", .line = 2, .column = 11},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 2, .column = 12}
                     });
    }


    TEST_F(LexerHappyPathTest, EmptyFile)
    {
        ExpectTokens("", {
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 1}
                     });
    }

    TEST_F(LexerHappyPathTest, OnlyComment)
    {
        ExpectTokens("// comment", {
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 11}
                     });
    }

    TEST_F(LexerHappyPathTest, TabsAndSpaces)
    {
        ExpectTokens("\t\t   ", {
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 6}
                     });
    }


    TEST_F(LexerHappyPathTest, MaxMunch1)
    {
        ExpectTokens("=", {
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 1},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 2}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch2)
    {
        ExpectTokens("==", {
                         {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 1},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 3}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch3)
    {
        ExpectTokens("===", {
                         {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 1},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 3},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 4}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch4)
    {
        ExpectTokens("====", {
                         {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 1},
                         {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 3},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 5}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch5)
    {
        ExpectTokens("!=!=", {
                         {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 1},
                         {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 3},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 5}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch6)
    {
        ExpectTokens("!=!==", {
                         {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 1},
                         {.type = TokenType::NotEquals, .lexeme = "!=", .line = 1, .column = 3},
                         {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 5},
                         {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 6}
                     });
    }

    // --- Integration & Multi-line Tests ---

    TEST_F(LexerHappyPathTest, MultilineIntegration)
    {
        std::string source_code = "let a = 1_000.50\nfunc main() { return a } // A comment\n@directive \"math\"";
        ExpectTokens(source_code, {
            {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
            {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
            {.type = TokenType::Number, .lexeme = "1_000.50", .line = 1, .column = 9},
            {.type = TokenType::Func, .lexeme = "func", .line = 2, .column = 1},
            {.type = TokenType::Identifier, .lexeme = "main", .line = 2, .column = 6},
            {.type = TokenType::LeftParen, .lexeme = "(", .line = 2, .column = 10},
            {.type = TokenType::RightParen, .lexeme = ")", .line = 2, .column = 11},
            {.type = TokenType::LeftBrace, .lexeme = "{", .line = 2, .column = 13},
            {.type = TokenType::Return, .lexeme = "return", .line = 2, .column = 15},
            {.type = TokenType::Identifier, .lexeme = "a", .line = 2, .column = 22},
            {.type = TokenType::RightBrace, .lexeme = "}", .line = 2, .column = 24},
            {.type = TokenType::At, .lexeme = "@", .line = 3, .column = 1},
            {.type = TokenType::Identifier, .lexeme = "directive", .line = 3, .column = 2},
            {.type = TokenType::String, .lexeme = "\"math\"", .line = 3, .column = 12},
            {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 18}
        });
    }

    TEST_F(LexerHappyPathTest, IgnoresWhitespaceAndComments)
    {
        const std::string code =
            "let a = 5\n"
            "// This is a comment\n"
            "    \t return a";

        ExpectTokens(code, {
            {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
            {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
            {.type = TokenType::Number, .lexeme = "5", .line = 1, .column = 9},
            {.type = TokenType::Return, .lexeme = "return", .line = 3, .column = 7},
            {.type = TokenType::Identifier, .lexeme = "a", .line = 3, .column = 14},
            {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 15}
        });
    }

    TEST_F(LexerHappyPathTest, ConsecutiveOperatorsWithoutSpaces)
    {
        ExpectTokens("if(a==-1)", {
            {.type = TokenType::If, .lexeme = "if", .line = 1, .column = 1},
            {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 3},
            {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 4},
            {.type = TokenType::Equals, .lexeme = "==", .line = 1, .column = 5},
            {.type = TokenType::Minus, .lexeme = "-", .line = 1, .column = 7},
            {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 8},
            {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 9},
            {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 10}
        });
    }

    TEST_F(LexerHappyPathTest, ValidatesTokenByteOffsets)
    {
        std::string source = "let sum = a + 42";
        auto ctx = std::make_shared<CompilerContext>();
        auto tokens = tokenize_code(source, true, ctx);

        // 'let': start=0, length=3
        EXPECT_EQ(tokens[0].start_offset, 0);
        EXPECT_EQ(tokens[0].length, 3);
        EXPECT_EQ(source.substr(tokens[0].start_offset, tokens[0].length), "let");

        // 'sum': start=4, length=3
        EXPECT_EQ(tokens[1].start_offset, 4);
        EXPECT_EQ(tokens[1].length, 3);
        EXPECT_EQ(source.substr(tokens[1].start_offset, tokens[1].length), "sum");

        // '=': start=8, length=1
        EXPECT_EQ(tokens[2].start_offset, 8);
        EXPECT_EQ(tokens[2].length, 1);
        EXPECT_EQ(source.substr(tokens[2].start_offset, tokens[2].length), "=");

        // 'a': start=10, length=1
        EXPECT_EQ(tokens[3].start_offset, 10);
        EXPECT_EQ(tokens[3].length, 1);
        EXPECT_EQ(source.substr(tokens[3].start_offset, tokens[3].length), "a");

        // '+': start=12, length=1
        EXPECT_EQ(tokens[4].start_offset, 12);
        EXPECT_EQ(tokens[4].length, 1);
        EXPECT_EQ(source.substr(tokens[4].start_offset, tokens[4].length), "+");

        // '42': start=14, length=2
        EXPECT_EQ(tokens[5].start_offset, 14);
        EXPECT_EQ(tokens[5].length, 2);
        EXPECT_EQ(source.substr(tokens[5].start_offset, tokens[5].length), "42");

        // EOF: start=16, length=0
        EXPECT_EQ(tokens[6].start_offset, 16);
        EXPECT_EQ(tokens[6].length, 0);
    }

    TEST_F(LexerHappyPathTest, CollectsCommentTriviaWithSpansAndOffsets)
    {
        std::string source =
            "// First comment\n"
            "let x = 1 // vs-lint:disable-line\n"
            "// Trailing TODO: fixme";

        auto ctx = std::make_shared<CompilerContext>();
        auto tokens = tokenize_code(source, true, ctx);

        // Tokens only contain code and EOF
        ASSERT_EQ(tokens.size(), 5); // let, x, =, 1, EOF

        const auto& comments = ctx->get_comments("test.vs");
        ASSERT_EQ(comments.size(), 3);

        // Comment 1: "// First comment"
        EXPECT_EQ(comments[0].text, "// First comment");
        EXPECT_EQ(comments[0].line, 1);
        EXPECT_EQ(comments[0].column, 1);
        EXPECT_EQ(comments[0].start_offset, 0);
        EXPECT_EQ(comments[0].length, 16);
        EXPECT_EQ(comments[0].span.line_start, 1);
        EXPECT_EQ(comments[0].span.column_start, 1);
        EXPECT_EQ(comments[0].span.line_end, 1);
        EXPECT_EQ(comments[0].span.column_end, 17);
        EXPECT_EQ(comments[0].span.start_offset, 0);
        EXPECT_EQ(comments[0].span.length, 16);
        EXPECT_EQ(comments[0].span.end_offset(), 16);

        // Comment 2: "// vs-lint:disable-line"
        size_t c2_offset = source.find("// vs-lint");
        EXPECT_EQ(comments[1].text, "// vs-lint:disable-line");
        EXPECT_EQ(comments[1].line, 2);
        EXPECT_EQ(comments[1].column, 11);
        EXPECT_EQ(comments[1].start_offset, c2_offset);
        EXPECT_EQ(comments[1].length, 23);

        // Comment 3: "// Trailing TODO: fixme"
        size_t c3_offset = source.find("// Trailing");
        EXPECT_EQ(comments[2].text, "// Trailing TODO: fixme");
        EXPECT_EQ(comments[2].line, 3);
        EXPECT_EQ(comments[2].column, 1);
        EXPECT_EQ(comments[2].start_offset, c3_offset);
        EXPECT_EQ(comments[2].length, 23);

        // Context general list also contains all comments
        EXPECT_EQ(ctx->comments.size(), 3);
    }
}
