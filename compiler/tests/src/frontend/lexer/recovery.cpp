#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "lexer_test_base.h"

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    class LexerRecoveryTest : public LexerTestBase
    {
    };

    TEST_F(LexerRecoveryTest, InvalidCharDollar)
    {
        ExpectLexerRecovery("let a = $",
            {{.code = E::InvalidCharacter, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 10}
            });
    }

    TEST_F(LexerRecoveryTest, InvalidCharAmpersand)
    {
        ExpectLexerRecovery("let a = &",
            {{.code = E::InvalidCharacter, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 10}
            });
    }

    TEST_F(LexerRecoveryTest, PercentageBeforeNumber)
    {
        ExpectLexerRecovery("x = %1",
            {{.code = E::InvalidCharacter, .line = 1, .column = 5}},
            {
                {.type = TokenType::Identifier, .lexeme = "x", .line = 1, .column = 1},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 3},
                {.type = TokenType::Number, .lexeme = "1", .line = 1, .column = 6},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 7}
            });
    }

    TEST_F(LexerRecoveryTest, UnterminatedDecimal1)
    {
        ExpectLexerRecovery("let a = 1.",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::Number, .lexeme = "1.", .line = 1, .column = 9},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 11}
            });
    }

    TEST_F(LexerRecoveryTest, UnterminatedDecimal2)
    {
        ExpectLexerRecovery("let a = 1_230.",
            {{.code = E::UnterminatedDecimal, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::Number, .lexeme = "1_230.", .line = 1, .column = 9},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
            });
    }

    TEST_F(LexerRecoveryTest, UnterminatedNumberAfterSeparator)
    {
        ExpectLexerRecovery("let a = 1_",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::Number, .lexeme = "1_", .line = 1, .column = 9},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 11}
            });
    }

    TEST_F(LexerRecoveryTest, AtDoubleUnderscoreInvalid)
    {
        ExpectLexerRecovery("let a = 1__000",
            {{.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::Number, .lexeme = "1_", .line = 1, .column = 9},
                {.type = TokenType::Identifier, .lexeme = "_000", .line = 1, .column = 11},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedString1)
    {
        ExpectLexerRecovery("let a = \"hello",
            {{.code = E::UnclosedString, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::String, .lexeme = "\"hello", .line = 1, .column = 9},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 15}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedString2)
    {
        ExpectLexerRecovery("let a = \"hello \n \n",
            {{.code = E::UnclosedString, .line = 1, .column = 9}},
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::String, .lexeme = "\"hello ", .line = 1, .column = 9},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 1}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedString3)
    {
        ExpectLexerRecovery("\"hello",
            {{.code = E::UnclosedString, .line = 1, .column = 1}},
            {
                {.type = TokenType::String, .lexeme = "\"hello", .line = 1, .column = 1},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 7}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedStringImport)
    {
        ExpectLexerRecovery("import \"file/path",
            {{.code = E::UnclosedString, .line = 1, .column = 8}},
            {
                {.type = TokenType::Import, .lexeme = "import", .line = 1, .column = 1},
                {.type = TokenType::String, .lexeme = "\"file/path", .line = 1, .column = 8},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 18}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedDocString1)
    {
        ExpectLexerRecovery(R"(func test() -> scalar { """"" return 1 })",
            {{.code = E::UnclosedString, .line = 1, .column = 25}},
            {
                {.type = TokenType::Func, .lexeme = "func", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "test", .line = 1, .column = 6},
                {.type = TokenType::LeftParen, .lexeme = "(", .line = 1, .column = 10},
                {.type = TokenType::RightParen, .lexeme = ")", .line = 1, .column = 11},
                {.type = TokenType::Arrow, .lexeme = "->", .line = 1, .column = 13},
                {.type = TokenType::Identifier, .lexeme = "scalar", .line = 1, .column = 16},
                {.type = TokenType::LeftBrace, .lexeme = "{", .line = 1, .column = 23},
                {.type = TokenType::DocString, .lexeme = R"(""""" return 1 })", .line = 1, .column = 25},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 41}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedDocString2)
    {
        ExpectLexerRecovery(R"(""""" return 1 })",
            {{.code = E::UnclosedString, .line = 1, .column = 1}},
            {
                {.type = TokenType::DocString, .lexeme = R"(""""" return 1 })", .line = 1, .column = 1},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 17}
            });
    }

    TEST_F(LexerRecoveryTest, MultipleInvalidCharacters)
    {
        ExpectLexerRecovery("let a = $\nlet b = ~\nlet c = \\",
            {
                {.code = E::InvalidCharacter, .line = 1, .column = 9},
                {.code = E::InvalidCharacter, .line = 2, .column = 9},
                {.code = E::InvalidCharacter, .line = 3, .column = 9}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::Let, .lexeme = "let", .line = 2, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "b", .line = 2, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 2, .column = 7},
                {.type = TokenType::Let, .lexeme = "let", .line = 3, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "c", .line = 3, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 3, .column = 7},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 10}
            });
    }

    TEST_F(LexerRecoveryTest, MixedTokenErrors)
    {
        ExpectLexerRecovery("let w = 12.\nlet x = .5\nlet y = \"unclosed string spanning to EOF",
            {
                {.code = E::UnterminatedDecimal, .line = 1, .column = 9},
                {.code = E::UnclosedString, .line = 3, .column = 9}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "w", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 7},
                {.type = TokenType::Number, .lexeme = "12.", .line = 1, .column = 9},
                {.type = TokenType::Let, .lexeme = "let", .line = 2, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "x", .line = 2, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 2, .column = 7},
                {.type = TokenType::Number, .lexeme = ".5", .line = 2, .column = 9},
                {.type = TokenType::Let, .lexeme = "let", .line = 3, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "y", .line = 3, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 3, .column = 7},
                {.type = TokenType::String, .lexeme = "\"unclosed string spanning to EOF", .line = 3, .column = 9},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 41}
            });
    }

    TEST_F(LexerRecoveryTest, SuccessiveInvalidCharacters)
    {
        ExpectLexerRecovery("$$$\n~~~",
            {
                {.code = E::InvalidCharacter, .line = 1, .column = 1},
                {.code = E::InvalidCharacter, .line = 1, .column = 2},
                {.code = E::InvalidCharacter, .line = 1, .column = 3},
                {.code = E::InvalidCharacter, .line = 2, .column = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 2},
                {.code = E::InvalidCharacter, .line = 2, .column = 3}
            },
            {
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 2, .column = 4}
            });
    }

    TEST_F(LexerRecoveryTest, InvalidUnderscoresInNumbers)
    {
        ExpectLexerRecovery("let num1 = 12__3\nlet num2 = 45._\nlet num3 = 100_",
            {
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 12},
                {.code = E::UnterminatedDecimal, .line = 2, .column = 12},
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 3, .column = 12}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "num1", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 10},
                {.type = TokenType::Number, .lexeme = "12_", .line = 1, .column = 12},
                {.type = TokenType::Identifier, .lexeme = "_3", .line = 1, .column = 15},
                {.type = TokenType::Let, .lexeme = "let", .line = 2, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "num2", .line = 2, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 2, .column = 10},
                {.type = TokenType::Number, .lexeme = "45.", .line = 2, .column = 12},
                {.type = TokenType::Identifier, .lexeme = "_", .line = 2, .column = 15},
                {.type = TokenType::Let, .lexeme = "let", .line = 3, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "num3", .line = 3, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 3, .column = 10},
                {.type = TokenType::Number, .lexeme = "100_", .line = 3, .column = 12},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 16}
            });
    }

    TEST_F(LexerRecoveryTest, LargeScaleStressTest)
    {
        std::string source =
            "\n\n"
            "let value = 100 \n"
            "let invalid1 = $  \n"
            "let partial = 0.\n"
            "let invalid2 = ~\n\n"
            "let unclosed = \"started\n";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter, .line = 4, .column = 16},
                {.code = E::UnterminatedDecimal, .line = 5, .column = 15},
                {.code = E::InvalidCharacter, .line = 6, .column = 16},
                {.code = E::UnclosedString, .line = 8, .column = 16}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 3, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "value", .line = 3, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 3, .column = 11},
                {.type = TokenType::Number, .lexeme = "100", .line = 3, .column = 13},
                {.type = TokenType::Let, .lexeme = "let", .line = 4, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "invalid1", .line = 4, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 4, .column = 14},
                {.type = TokenType::Let, .lexeme = "let", .line = 5, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "partial", .line = 5, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 5, .column = 13},
                {.type = TokenType::Number, .lexeme = "0.", .line = 5, .column = 15},
                {.type = TokenType::Let, .lexeme = "let", .line = 6, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "invalid2", .line = 6, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 6, .column = 14},
                {.type = TokenType::Let, .lexeme = "let", .line = 8, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "unclosed", .line = 8, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 8, .column = 14},
                {.type = TokenType::String, .lexeme = "\"started", .line = 8, .column = 16},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 9, .column = 1}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedDocString)
    {
        std::string source =
            "let doc = \"\"\"This is a docstring\n"
            "that spans multiple lines\n"
            "but never closes properly...";

        ExpectLexerRecovery(source,
            {
                {.code = E::UnclosedString, .line = 1, .column = 11}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "doc", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 9},
                {.type = TokenType::DocString, .lexeme = "\"\"\"This is a docstring\nthat spans multiple lines\nbut never closes properly...", .line = 1, .column = 11},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 30}
            });
    }

    TEST_F(LexerRecoveryTest, HiddenLexicalErrorsInMath)
    {
        ExpectLexerRecovery("let result = 10 + .5 * 100_ - \"unclosed",
            {
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 1, .column = 24},
                {.code = E::UnclosedString, .line = 1, .column = 31}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "result", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 12},
                {.type = TokenType::Number, .lexeme = "10", .line = 1, .column = 14},
                {.type = TokenType::Plus, .lexeme = "+", .line = 1, .column = 17},
                {.type = TokenType::Number, .lexeme = ".5", .line = 1, .column = 19},
                {.type = TokenType::Star, .lexeme = "*", .line = 1, .column = 22},
                {.type = TokenType::Number, .lexeme = "100_", .line = 1, .column = 24},
                {.type = TokenType::Minus, .lexeme = "-", .line = 1, .column = 29},
                {.type = TokenType::String, .lexeme = "\"unclosed", .line = 1, .column = 31},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 1, .column = 40}
            });
    }

    TEST_F(LexerRecoveryTest, InterleavedValidAndInvalid)
    {
        std::string source =
            "let valid1 = 100\n"
            "let bad1 = $\n"
            "let valid2 = \"test\"\n"
            "let bad2 = ~\n"
            "let valid3 = .99\n"
            "let valid4 = 0.99";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter, .line = 2, .column = 12},
                {.code = E::InvalidCharacter, .line = 4, .column = 12}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 1, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "valid1", .line = 1, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 1, .column = 12},
                {.type = TokenType::Number, .lexeme = "100", .line = 1, .column = 14},
                {.type = TokenType::Let, .lexeme = "let", .line = 2, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "bad1", .line = 2, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 2, .column = 10},
                {.type = TokenType::Let, .lexeme = "let", .line = 3, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "valid2", .line = 3, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 3, .column = 12},
                {.type = TokenType::String, .lexeme = "\"test\"", .line = 3, .column = 14},
                {.type = TokenType::Let, .lexeme = "let", .line = 4, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "bad2", .line = 4, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 4, .column = 10},
                {.type = TokenType::Let, .lexeme = "let", .line = 5, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "valid3", .line = 5, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 5, .column = 12},
                {.type = TokenType::Number, .lexeme = ".99", .line = 5, .column = 14},
                {.type = TokenType::Let, .lexeme = "let", .line = 6, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "valid4", .line = 6, .column = 5},
                {.type = TokenType::Assign, .lexeme = "=", .line = 6, .column = 12},
                {.type = TokenType::Number, .lexeme = "0.99", .line = 6, .column = 14},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 6, .column = 18}
            });
    }

    TEST_F(LexerRecoveryTest, RapidFireCorruptions)
    {
        std::string source = "\"unclosed 1\n12.\n\"unclosed 2\n.9\n1_a";

        ExpectLexerRecovery(source,
            {
                {.code = E::UnclosedString, .line = 1, .column = 1},
                {.code = E::UnterminatedDecimal, .line = 2, .column = 1},
                {.code = E::UnclosedString, .line = 3, .column = 1},
                {.code = E::TrailingSeparatorInNumberLiteral, .line = 5, .column = 1}
            },
            {
                {.type = TokenType::String, .lexeme = "\"unclosed 1", .line = 1, .column = 1},
                {.type = TokenType::Number, .lexeme = "12.", .line = 2, .column = 1},
                {.type = TokenType::String, .lexeme = "\"unclosed 2", .line = 3, .column = 1},
                {.type = TokenType::Number, .lexeme = ".9", .line = 4, .column = 1},
                {.type = TokenType::Number, .lexeme = "1_", .line = 5, .column = 1},
                {.type = TokenType::Identifier, .lexeme = "a", .line = 5, .column = 3},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 5, .column = 4}
            });
    }

    TEST_F(LexerRecoveryTest, WhitespaceAndIndentationTracking)
    {
        std::string source = "\n\n\t\tlet spaced =   $\n    let padded = .123\n    let carriage = 12.\n";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter, .line = 3, .column = 18},
                {.code = E::UnterminatedDecimal, .line = 5, .column = 20}
            },
            {
                {.type = TokenType::Let, .lexeme = "let", .line = 3, .column = 3},
                {.type = TokenType::Identifier, .lexeme = "spaced", .line = 3, .column = 7},
                {.type = TokenType::Assign, .lexeme = "=", .line = 3, .column = 14},
                {.type = TokenType::Let, .lexeme = "let", .line = 4, .column = 5},
                {.type = TokenType::Identifier, .lexeme = "padded", .line = 4, .column = 9},
                {.type = TokenType::Assign, .lexeme = "=", .line = 4, .column = 16},
                {.type = TokenType::Number, .lexeme = ".123", .line = 4, .column = 18},
                {.type = TokenType::Let, .lexeme = "let", .line = 5, .column = 5},
                {.type = TokenType::Identifier, .lexeme = "carriage", .line = 5, .column = 9},
                {.type = TokenType::Assign, .lexeme = "=", .line = 5, .column = 18},
                {.type = TokenType::Number, .lexeme = "12.", .line = 5, .column = 20},
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 6, .column = 1}
            });
    }

    TEST_F(LexerRecoveryTest, PureGarbageFile)
    {
        std::string source = "?\\`\n~|\n$$$";

        ExpectLexerRecovery(source,
            {
                {.code = E::InvalidCharacter, .line = 1, .column = 1},
                {.code = E::InvalidCharacter, .line = 1, .column = 2},
                {.code = E::InvalidCharacter, .line = 1, .column = 3},
                {.code = E::InvalidCharacter, .line = 2, .column = 1},
                {.code = E::InvalidCharacter, .line = 2, .column = 2},
                {.code = E::InvalidCharacter, .line = 3, .column = 1},
                {.code = E::InvalidCharacter, .line = 3, .column = 2},
                {.code = E::InvalidCharacter, .line = 3, .column = 3}
            },
            {
                {.type = TokenType::EndOfFile, .lexeme = "", .line = 3, .column = 4}
            });
    }
}
