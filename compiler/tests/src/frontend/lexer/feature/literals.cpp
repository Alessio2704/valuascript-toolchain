#include <gtest/gtest.h>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerLiteralTest : public LexerTestBase
    {
    };

    TEST_F(LexerLiteralTest, ZeroInteger)
    {
        ExpectTokens("0", {
            {.type = TokenType::Number, .lexeme = "0", .line = 1, .column = 1, .start_offset = 0, .length = 1}
        });
    }

    TEST_F(LexerLiteralTest, PositiveIntegers)
    {
        ExpectTokens("42", {
            {.type = TokenType::Number, .lexeme = "42", .line = 1, .column = 1, .start_offset = 0, .length = 2}
        });

        ExpectTokens("1000", {
            {.type = TokenType::Number, .lexeme = "1000", .line = 1, .column = 1, .start_offset = 0, .length = 4}
        });
    }

    TEST_F(LexerLiteralTest, IntegersWithUnderscoreSeparators)
    {
        ExpectTokens("1_000_000", {
            {.type = TokenType::Number, .lexeme = "1_000_000", .line = 1, .column = 1, .start_offset = 0, .length = 9}
        });

        ExpectTokens("1_0", {
            {.type = TokenType::Number, .lexeme = "1_0", .line = 1, .column = 1, .start_offset = 0, .length = 3}
        });
    }

    TEST_F(LexerLiteralTest, StandardDecimals)
    {
        ExpectTokens("0.0", {
            {.type = TokenType::Number, .lexeme = "0.0", .line = 1, .column = 1, .start_offset = 0, .length = 3}
        });

        ExpectTokens("1000.5", {
            {.type = TokenType::Number, .lexeme = "1000.5", .line = 1, .column = 1, .start_offset = 0, .length = 6}
        });

        ExpectTokens("3.14159", {
            {.type = TokenType::Number, .lexeme = "3.14159", .line = 1, .column = 1, .start_offset = 0, .length = 7}
        });
    }

    TEST_F(LexerLiteralTest, LeadingDotDecimals)
    {
        ExpectTokens(".5", {
            {.type = TokenType::Number, .lexeme = ".5", .line = 1, .column = 1, .start_offset = 0, .length = 2}
        });

        ExpectTokens(".12345", {
            {.type = TokenType::Number, .lexeme = ".12345", .line = 1, .column = 1, .start_offset = 0, .length = 6}
        });
    }

    TEST_F(LexerLiteralTest, DecimalsWithUnderscoreSeparators)
    {
        ExpectTokens("1_000.50_000", {
            {.type = TokenType::Number, .lexeme = "1_000.50_000", .line = 1, .column = 1, .start_offset = 0, .length = 12}
        });
    }

    TEST_F(LexerLiteralTest, IntegerPercentages)
    {
        ExpectTokens("0%", {
            {.type = TokenType::PercentageLiteral, .lexeme = "0%", .line = 1, .column = 1, .start_offset = 0, .length = 2}
        });

        ExpectTokens("50%", {
            {.type = TokenType::PercentageLiteral, .lexeme = "50%", .line = 1, .column = 1, .start_offset = 0, .length = 3}
        });

        ExpectTokens("1_000%", {
            {.type = TokenType::PercentageLiteral, .lexeme = "1_000%", .line = 1, .column = 1, .start_offset = 0, .length = 6}
        });
    }

    TEST_F(LexerLiteralTest, DecimalPercentages)
    {
        ExpectTokens("100.5%", {
            {.type = TokenType::PercentageLiteral, .lexeme = "100.5%", .line = 1, .column = 1, .start_offset = 0, .length = 6}
        });

        ExpectTokens(".5%", {
            {.type = TokenType::PercentageLiteral, .lexeme = ".5%", .line = 1, .column = 1, .start_offset = 0, .length = 3}
        });
    }

    TEST_F(LexerLiteralTest, EmptyString)
    {
        ExpectTokens("\"\"", {
            {.type = TokenType::String, .lexeme = "\"\"", .line = 1, .column = 1, .start_offset = 0, .length = 2}
        });
    }

    TEST_F(LexerLiteralTest, SimpleStrings)
    {
        ExpectTokens("\"hello\"", {
            {.type = TokenType::String, .lexeme = "\"hello\"", .line = 1, .column = 1, .start_offset = 0, .length = 7}
        });

        ExpectTokens("\"hello world\"", {
            {.type = TokenType::String, .lexeme = "\"hello world\"", .line = 1, .column = 1, .start_offset = 0, .length = 13}
        });
    }

    TEST_F(LexerLiteralTest, DocStrings)
    {
        ExpectTokens("\"\"\"\"\"\"", {
            {.type = TokenType::DocString, .lexeme = "\"\"\"\"\"\"", .line = 1, .column = 1, .start_offset = 0, .length = 6}
        });

        ExpectTokens("\"\"\"docstring text\"\"\"", {
            {.type = TokenType::DocString, .lexeme = "\"\"\"docstring text\"\"\"", .line = 1, .column = 1, .start_offset = 0, .length = 20}
        });

        std::string multiline_doc = "\"\"\"line 1\nline 2\nline 3\"\"\"";
        ExpectTokens(multiline_doc, {
            {.type = TokenType::DocString, .lexeme = multiline_doc, .line = 1, .column = 1, .start_offset = 0, .length = multiline_doc.length()}
        });
    }

    TEST_F(LexerLiteralTest, BooleanLiterals)
    {
        ExpectTokens("true", {
            {.type = TokenType::True, .lexeme = "true", .line = 1, .column = 1, .start_offset = 0, .length = 4}
        });

        ExpectTokens("false", {
            {.type = TokenType::False, .lexeme = "false", .line = 1, .column = 1, .start_offset = 0, .length = 5}
        });
    }
}
