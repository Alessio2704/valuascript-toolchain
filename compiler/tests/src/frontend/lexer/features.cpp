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
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Number, "1000", 1, 9},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, Number1)
    {
        ExpectTokens("1000", {
                         {TokenType::Number, "1000", 1, 1},
                         {TokenType::EndOfFile, "", 1, 5}
                     });
    }

    TEST_F(LexerHappyPathTest, Number2)
    {
        ExpectTokens("1000.5", {
                         {TokenType::Number, "1000.5", 1, 1},
                         {TokenType::EndOfFile, "", 1, 7}
                     });
    }

    TEST_F(LexerHappyPathTest, Number3)
    {
        ExpectTokens("1_000.5", {
                         {TokenType::Number, "1_000.5", 1, 1},
                         {TokenType::EndOfFile, "", 1, 8}
                     });
    }

    TEST_F(LexerHappyPathTest, Number4)
    {
        ExpectTokens("0.5", {
                         {TokenType::Number, "0.5", 1, 1},
                         {TokenType::EndOfFile, "", 1, 4}
                     });
    }

    TEST_F(LexerHappyPathTest, NumberPercentage1)
    {
        ExpectTokens("let a = 5%", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::PercentageLiteral, "5%", 1, 9},
                         {TokenType::EndOfFile, "", 1, 11}
                     });
    }

    TEST_F(LexerHappyPathTest, NumberPercentage2)
    {
        ExpectTokens("let a = 0.5%", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::PercentageLiteral, "0.5%", 1, 9},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, NumberPercentage3)
    {
        ExpectTokens("let a = 1_000.5%", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::PercentageLiteral, "1_000.5%", 1, 9},
                         {TokenType::EndOfFile, "", 1, 17}
                     });
    }

    TEST_F(LexerHappyPathTest, String)
    {
        ExpectTokens("let a = \"string\"", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::String, "\"string\"", 1, 9},
                         {TokenType::EndOfFile, "", 1, 17}
                     });
    }

    TEST_F(LexerHappyPathTest, VectorLiteral)
    {
        ExpectTokens("let a = [1,2,3]", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftBracket, "[", 1, 9},
                         {TokenType::Number, "1", 1, 10},
                         {TokenType::Comma, ",", 1, 11},
                         {TokenType::Number, "2", 1, 12},
                         {TokenType::Comma, ",", 1, 13},
                         {TokenType::Number, "3", 1, 14},
                         {TokenType::RightBracket, "]", 1, 15},
                         {TokenType::EndOfFile, "", 1, 16}
                     });
    }

    TEST_F(LexerHappyPathTest, Boolean)
    {
        ExpectTokens("let a = true", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::True, "true", 1, 9},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, Cname1)
    {
        ExpectTokens("let _a = false", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "_a", 1, 5},
                         {TokenType::Assign, "=", 1, 8},
                         {TokenType::False, "false", 1, 10},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, IdentifierContainingKeyword)
    {
        ExpectTokens("let ifthenelse = 1", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "ifthenelse", 1, 5},
                         {TokenType::Assign, "=", 1, 16},
                         {TokenType::Number, "1", 1, 18},
                         {TokenType::EndOfFile, "", 1, 19}
                     });
    }

    TEST_F(LexerHappyPathTest, FunctionCall)
    {
        ExpectTokens("let a = some_func()", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "some_func", 1, 9},
                         {TokenType::LeftParen, "(", 1, 18},
                         {TokenType::RightParen, ")", 1, 19},
                         {TokenType::EndOfFile, "", 1, 20}
                     });
    }

    TEST_F(LexerHappyPathTest, MultiAssignment)
    {
        ExpectTokens("let a, b = some_func()", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Comma, ",", 1, 6},
                         {TokenType::Identifier, "b", 1, 8},
                         {TokenType::Assign, "=", 1, 10},
                         {TokenType::Identifier, "some_func", 1, 12},
                         {TokenType::LeftParen, "(", 1, 21},
                         {TokenType::RightParen, ")", 1, 22},
                         {TokenType::EndOfFile, "", 1, 23}
                     });
    }

    TEST_F(LexerHappyPathTest, ConditionalExpression)
    {
        ExpectTokens("let a = if true then 10 else 4", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::If, "if", 1, 9},
                         {TokenType::True, "true", 1, 12},
                         {TokenType::Then, "then", 1, 17},
                         {TokenType::Number, "10", 1, 22},
                         {TokenType::Else, "else", 1, 25},
                         {TokenType::Number, "4", 1, 30},
                         {TokenType::EndOfFile, "", 1, 31}
                     });
    }

    TEST_F(LexerHappyPathTest, DictLiteral)
    {
        ExpectTokens("let a = {b: 1}", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftBrace, "{", 1, 9},
                         {TokenType::Identifier, "b", 1, 10},
                         {TokenType::Colon, ":", 1, 11},
                         {TokenType::Number, "1", 1, 13},
                         {TokenType::RightBrace, "}", 1, 14},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, DictLiteralSelf)
    {
        ExpectTokens("let a = {b: self.c}", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftBrace, "{", 1, 9},
                         {TokenType::Identifier, "b", 1, 10},
                         {TokenType::Colon, ":", 1, 11},
                         {TokenType::Self, "self", 1, 13},
                         {TokenType::Dot, ".", 1, 17},
                         {TokenType::Identifier, "c", 1, 18},
                         {TokenType::RightBrace, "}", 1, 19},
                         {TokenType::EndOfFile, "", 1, 20}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorOr)
    {
        ExpectTokens("let a = x or y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::Or, "or", 1, 11},
                         {TokenType::Identifier, "y", 1, 14},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorAnd)
    {
        ExpectTokens("let a = x and y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::And, "and", 1, 11},
                         {TokenType::Identifier, "y", 1, 15},
                         {TokenType::EndOfFile, "", 1, 16}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorNot)
    {
        ExpectTokens("let a = not x", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Not, "not", 1, 9},
                         {TokenType::Identifier, "x", 1, 13},
                         {TokenType::EndOfFile, "", 1, 14}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorEq)
    {
        ExpectTokens("let a = x == y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::Equals, "==", 1, 11},
                         {TokenType::Identifier, "y", 1, 14},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorNeq)
    {
        ExpectTokens("let a = x != y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::NotEquals, "!=", 1, 11},
                         {TokenType::Identifier, "y", 1, 14},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorGt)
    {
        ExpectTokens("let a = x > y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::Greater, ">", 1, 11},
                         {TokenType::Identifier, "y", 1, 13},
                         {TokenType::EndOfFile, "", 1, 14}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorLt)
    {
        ExpectTokens("let a = x < y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::Less, "<", 1, 11},
                         {TokenType::Identifier, "y", 1, 13},
                         {TokenType::EndOfFile, "", 1, 14}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorGte)
    {
        ExpectTokens("let a = x >= y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::GreaterEqual, ">=", 1, 11},
                         {TokenType::Identifier, "y", 1, 14},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorLte)
    {
        ExpectTokens("let a = x <= y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::LessEqual, "<=", 1, 11},
                         {TokenType::Identifier, "y", 1, 14},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, OperatorPow)
    {
        ExpectTokens("let a = x^y", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::Caret, "^", 1, 10},
                         {TokenType::Identifier, "y", 1, 11},
                         {TokenType::EndOfFile, "", 1, 12}
                     });
    }


    TEST_F(LexerHappyPathTest, AccessVectorElement)
    {
        ExpectTokens("let a = x[1]", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::LeftBracket, "[", 1, 10},
                         {TokenType::Number, "1", 1, 11},
                         {TokenType::RightBracket, "]", 1, 12},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, DeleteVectorElement)
    {
        ExpectTokens("let a = x[:1]", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Identifier, "x", 1, 9},
                         {TokenType::LeftBracket, "[", 1, 10},
                         {TokenType::Colon, ":", 1, 11},
                         {TokenType::Number, "1", 1, 12},
                         {TokenType::RightBracket, "]", 1, 13},
                         {TokenType::EndOfFile, "", 1, 14}
                     });
    }

    TEST_F(LexerHappyPathTest, ParenthesisInAssignment)
    {
        ExpectTokens("let a = (x + y) * z", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftParen, "(", 1, 9},
                         {TokenType::Identifier, "x", 1, 10},
                         {TokenType::Plus, "+", 1, 12},
                         {TokenType::Identifier, "y", 1, 14},
                         {TokenType::RightParen, ")", 1, 15},
                         {TokenType::Star, "*", 1, 17},
                         {TokenType::Identifier, "z", 1, 19},
                         {TokenType::EndOfFile, "", 1, 20}
                     });
    }


    TEST_F(LexerHappyPathTest, DirectiveIterations)
    {
        ExpectTokens("#iterations = 1000", {
                         {TokenType::Hash, "#", 1, 1},
                         {TokenType::Identifier, "iterations", 1, 2},
                         {TokenType::Assign, "=", 1, 13},
                         {TokenType::Number, "1000", 1, 15},
                         {TokenType::EndOfFile, "", 1, 19}
                     });
    }

    TEST_F(LexerHappyPathTest, DirectiveOutput)
    {
        ExpectTokens("#output = my_var", {
                         {TokenType::Hash, "#", 1, 1},
                         {TokenType::Identifier, "output", 1, 2},
                         {TokenType::Assign, "=", 1, 9},
                         {TokenType::Identifier, "my_var", 1, 11},
                         {TokenType::EndOfFile, "", 1, 17}
                     });
    }

    TEST_F(LexerHappyPathTest, DirectiveOutputFile)
    {
        ExpectTokens("#output_file = \"results.csv\"", {
                         {TokenType::Hash, "#", 1, 1},
                         {TokenType::Identifier, "output_file", 1, 2},
                         {TokenType::Assign, "=", 1, 14},
                         {TokenType::String, "\"results.csv\"", 1, 16},
                         {TokenType::EndOfFile, "", 1, 29}
                     });
    }

    TEST_F(LexerHappyPathTest, DirectiveModule)
    {
        ExpectTokens("#module", {
                         {TokenType::Hash, "#", 1, 1},
                         {TokenType::Identifier, "module", 1, 2},
                         {TokenType::EndOfFile, "", 1, 8}
                     });
    }


    TEST_F(LexerHappyPathTest, ImportStatement)
    {
        ExpectTokens("import \"file/path\"", {
                         {TokenType::Import, "import", 1, 1},
                         {TokenType::String, "\"file/path\"", 1, 8},
                         {TokenType::EndOfFile, "", 1, 19}
                     });
    }


    TEST_F(LexerHappyPathTest, TypealiasStatement)
    {
        ExpectTokens("typealias Container = vector<int>", {
                         {TokenType::Typealias, "typealias", 1, 1},
                         {TokenType::Identifier, "Container", 1, 11},
                         {TokenType::Assign, "=", 1, 21},
                         {TokenType::Identifier, "vector", 1, 23},
                         {TokenType::Less, "<", 1, 29},
                         {TokenType::Identifier, "int", 1, 30},
                         {TokenType::Greater, ">", 1, 33},
                         {TokenType::EndOfFile, "", 1, 34}
                     });
    }


    TEST_F(LexerHappyPathTest, StructDefinition)
    {
        ExpectTokens("struct Assumption { cagr: Decimal }", {
                         {TokenType::Struct, "struct", 1, 1},
                         {TokenType::Identifier, "Assumption", 1, 8},
                         {TokenType::LeftBrace, "{", 1, 19},
                         {TokenType::Identifier, "cagr", 1, 21},
                         {TokenType::Colon, ":", 1, 25},
                         {TokenType::Identifier, "Decimal", 1, 27},
                         {TokenType::RightBrace, "}", 1, 35},
                         {TokenType::EndOfFile, "", 1, 36}
                     });
    }


    TEST_F(LexerHappyPathTest, MemberAccess1)
    {
        ExpectTokens("model.cagr", {
                         {TokenType::Identifier, "model", 1, 1},
                         {TokenType::Dot, ".", 1, 6},
                         {TokenType::Identifier, "cagr", 1, 7},
                         {TokenType::EndOfFile, "", 1, 11}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccessUnderscoreIdentifier1)
    {
        ExpectTokens("_.prop", {
                         {TokenType::Identifier, "_", 1, 1},
                         {TokenType::Dot, ".", 1, 2},
                         {TokenType::Identifier, "prop", 1, 3},
                         {TokenType::EndOfFile, "", 1, 7}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccessUnderscoreIdentifier2)
    {
        ExpectTokens("_0.prop", {
                         {TokenType::Identifier, "_0", 1, 1},
                         {TokenType::Dot, ".", 1, 3},
                         {TokenType::Identifier, "prop", 1, 4},
                         {TokenType::EndOfFile, "", 1, 8}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess2)
    {
        ExpectTokens("let a = [].5", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftBracket, "[", 1, 9},
                         {TokenType::RightBracket, "]", 1, 10},
                         {TokenType::Number, ".5", 1, 11},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess3)
    {
        ExpectTokens("let a = ().5", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftParen, "(", 1, 9},
                         {TokenType::RightParen, ")", 1, 10},
                         {TokenType::Number, ".5", 1, 11},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess4)
    {
        ExpectTokens("let a = {}.5", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftBrace, "{", 1, 9},
                         {TokenType::RightBrace, "}", 1, 10},
                         {TokenType::Number, ".5", 1, 11},
                         {TokenType::EndOfFile, "", 1, 13}
                     });
    }

    TEST_F(LexerHappyPathTest, MemberAccess5)
    {
        ExpectTokens("let a = {a: 1}.5", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::LeftBrace, "{", 1, 9},
                         {TokenType::Identifier, "a", 1, 10},
                         {TokenType::Colon, ":", 1, 11},
                         {TokenType::Number, "1", 1, 13},
                         {TokenType::RightBrace, "}", 1, 14},
                         {TokenType::Number, ".5", 1, 15},
                         {TokenType::EndOfFile, "", 1, 17}
                     });
    }


    TEST_F(LexerHappyPathTest, AtSeparator)
    {
        ExpectTokens("let a = 1_000", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Number, "1_000", 1, 9},
                         {TokenType::EndOfFile, "", 1, 14}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloat)
    {
        ExpectTokens("let a = 1.3242", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Number, "1.3242", 1, 9},
                         {TokenType::EndOfFile, "", 1, 15}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloatAndSeparator)
    {
        ExpectTokens("let a = 1_103.3_242", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Number, "1_103.3_242", 1, 9},
                         {TokenType::EndOfFile, "", 1, 20}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloatSigned1)
    {
        ExpectTokens("let a = +1_103.3_242", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Plus, "+", 1, 9},
                         {TokenType::Number, "1_103.3_242", 1, 10},
                         {TokenType::EndOfFile, "", 1, 21}
                     });
    }

    TEST_F(LexerHappyPathTest, AtFloatSigned2)
    {
        ExpectTokens("let a = -1_103.3_242", {
                         {TokenType::Let, "let", 1, 1},
                         {TokenType::Identifier, "a", 1, 5},
                         {TokenType::Assign, "=", 1, 7},
                         {TokenType::Minus, "-", 1, 9},
                         {TokenType::Number, "1_103.3_242", 1, 10},
                         {TokenType::EndOfFile, "", 1, 21}
                     });
    }


    TEST_F(LexerHappyPathTest, FuncBasic)
    {
        ExpectTokens("func test() -> scalar { return 1 }", {
                         {TokenType::Func, "func", 1, 1},
                         {TokenType::Identifier, "test", 1, 6},
                         {TokenType::LeftParen, "(", 1, 10},
                         {TokenType::RightParen, ")", 1, 11},
                         {TokenType::Arrow, "->", 1, 13},
                         {TokenType::Identifier, "scalar", 1, 16},
                         {TokenType::LeftBrace, "{", 1, 23},
                         {TokenType::Return, "return", 1, 25},
                         {TokenType::Number, "1", 1, 32},
                         {TokenType::RightBrace, "}", 1, 34},
                         {TokenType::EndOfFile, "", 1, 35}
                     });
    }

    TEST_F(LexerHappyPathTest, FuncParams)
    {
        ExpectTokens("func test(a: scalar) -> scalar { return 1 }", {
                         {TokenType::Func, "func", 1, 1},
                         {TokenType::Identifier, "test", 1, 6},
                         {TokenType::LeftParen, "(", 1, 10},
                         {TokenType::Identifier, "a", 1, 11},
                         {TokenType::Colon, ":", 1, 12},
                         {TokenType::Identifier, "scalar", 1, 14},
                         {TokenType::RightParen, ")", 1, 20},
                         {TokenType::Arrow, "->", 1, 22},
                         {TokenType::Identifier, "scalar", 1, 25},
                         {TokenType::LeftBrace, "{", 1, 32},
                         {TokenType::Return, "return", 1, 34},
                         {TokenType::Number, "1", 1, 41},
                         {TokenType::RightBrace, "}", 1, 43},
                         {TokenType::EndOfFile, "", 1, 44}
                     });
    }

    TEST_F(LexerHappyPathTest, FuncTupleReturn)
    {
        ExpectTokens("func test() -> (scalar, vector) { return 1 }", {
                         {TokenType::Func, "func", 1, 1},
                         {TokenType::Identifier, "test", 1, 6},
                         {TokenType::LeftParen, "(", 1, 10},
                         {TokenType::RightParen, ")", 1, 11},
                         {TokenType::Arrow, "->", 1, 13},
                         {TokenType::LeftParen, "(", 1, 16},
                         {TokenType::Identifier, "scalar", 1, 17},
                         {TokenType::Comma, ",", 1, 23},
                         {TokenType::Identifier, "vector", 1, 25},
                         {TokenType::RightParen, ")", 1, 31},
                         {TokenType::LeftBrace, "{", 1, 33},
                         {TokenType::Return, "return", 1, 35},
                         {TokenType::Number, "1", 1, 42},
                         {TokenType::RightBrace, "}", 1, 44},
                         {TokenType::EndOfFile, "", 1, 45}
                     });
    }

    TEST_F(LexerHappyPathTest, FuncDocstring)
    {
        ExpectTokens("func test() -> scalar { \"\"\"Docs\"\"\" \n return 1 }", {
                         {TokenType::Func, "func", 1, 1},
                         {TokenType::Identifier, "test", 1, 6},
                         {TokenType::LeftParen, "(", 1, 10},
                         {TokenType::RightParen, ")", 1, 11},
                         {TokenType::Arrow, "->", 1, 13},
                         {TokenType::Identifier, "scalar", 1, 16},
                         {TokenType::LeftBrace, "{", 1, 23},
                         {TokenType::DocString, "\"\"\"Docs\"\"\"", 1, 25},
                         {TokenType::Return, "return", 2, 2},
                         {TokenType::Number, "1", 2, 9},
                         {TokenType::RightBrace, "}", 2, 11},
                         {TokenType::EndOfFile, "", 2, 12}
                     });
    }


    TEST_F(LexerHappyPathTest, EmptyFile)
    {
        ExpectTokens("", {
                         {TokenType::EndOfFile, "", 1, 1}
                     });
    }

    TEST_F(LexerHappyPathTest, OnlyComment)
    {
        ExpectTokens("// comment", {
                         {TokenType::EndOfFile, "", 1, 11}
                     });
    }

    TEST_F(LexerHappyPathTest, TabsAndSpaces)
    {
        ExpectTokens("\t\t   ", {
                         {TokenType::EndOfFile, "", 1, 6}
                     });
    }


    TEST_F(LexerHappyPathTest, MaxMunch1)
    {
        ExpectTokens("=", {
                         {TokenType::Assign, "=", 1, 1},
                         {TokenType::EndOfFile, "", 1, 2}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch2)
    {
        ExpectTokens("==", {
                         {TokenType::Equals, "==", 1, 1},
                         {TokenType::EndOfFile, "", 1, 3}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch3)
    {
        ExpectTokens("===", {
                         {TokenType::Equals, "==", 1, 1},
                         {TokenType::Assign, "=", 1, 3},
                         {TokenType::EndOfFile, "", 1, 4}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch4)
    {
        ExpectTokens("====", {
                         {TokenType::Equals, "==", 1, 1},
                         {TokenType::Equals, "==", 1, 3},
                         {TokenType::EndOfFile, "", 1, 5}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch5)
    {
        ExpectTokens("!=!=", {
                         {TokenType::NotEquals, "!=", 1, 1},
                         {TokenType::NotEquals, "!=", 1, 3},
                         {TokenType::EndOfFile, "", 1, 5}
                     });
    }

    TEST_F(LexerHappyPathTest, MaxMunch6)
    {
        ExpectTokens("!=!==", {
                         {TokenType::NotEquals, "!=", 1, 1},
                         {TokenType::NotEquals, "!=", 1, 3},
                         {TokenType::Assign, "=", 1, 5},
                         {TokenType::EndOfFile, "", 1, 6}
                     });
    }

    // --- Integration & Multi-line Tests ---

    TEST_F(LexerHappyPathTest, MultilineIntegration)
    {
        std::string source_code = "let a = 1_000.50\nfunc main() { return a } // A comment\n@directive \"math\"";
        ExpectTokens(source_code, {
            {TokenType::Let, "let", 1, 1},
            {TokenType::Identifier, "a", 1, 5},
            {TokenType::Assign, "=", 1, 7},
            {TokenType::Number, "1_000.50", 1, 9},
            {TokenType::Func, "func", 2, 1},
            {TokenType::Identifier, "main", 2, 6},
            {TokenType::LeftParen, "(", 2, 10},
            {TokenType::RightParen, ")", 2, 11},
            {TokenType::LeftBrace, "{", 2, 13},
            {TokenType::Return, "return", 2, 15},
            {TokenType::Identifier, "a", 2, 22},
            {TokenType::RightBrace, "}", 2, 24},
            {TokenType::At, "@", 3, 1},
            {TokenType::Identifier, "directive", 3, 2},
            {TokenType::String, "\"math\"", 3, 12},
            {TokenType::EndOfFile, "", 3, 18}
        });
    }

    TEST_F(LexerHappyPathTest, IgnoresWhitespaceAndComments)
    {
        const std::string code =
            "let a = 5\n"
            "// This is a comment\n"
            "    \t return a";

        ExpectTokens(code, {
            {TokenType::Let, "let", 1, 1},
            {TokenType::Identifier, "a", 1, 5},
            {TokenType::Assign, "=", 1, 7},
            {TokenType::Number, "5", 1, 9},
            {TokenType::Return, "return", 3, 7},
            {TokenType::Identifier, "a", 3, 14},
            {TokenType::EndOfFile, "", 3, 15}
        });
    }

    TEST_F(LexerHappyPathTest, ConsecutiveOperatorsWithoutSpaces)
    {
        ExpectTokens("if(a==-1)", {
            {TokenType::If, "if", 1, 1},
            {TokenType::LeftParen, "(", 1, 3},
            {TokenType::Identifier, "a", 1, 4},
            {TokenType::Equals, "==", 1, 5},
            {TokenType::Minus, "-", 1, 7},
            {TokenType::Number, "1", 1, 8},
            {TokenType::RightParen, ")", 1, 9},
            {TokenType::EndOfFile, "", 1, 10}
        });
    }
}
