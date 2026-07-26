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
            {{E::InvalidCharacter, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::EndOfFile, "", 1, 10}
            });
    }

    TEST_F(LexerRecoveryTest, InvalidCharAmpersand)
    {
        ExpectLexerRecovery("let a = &",
            {{E::InvalidCharacter, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::EndOfFile, "", 1, 10}
            });
    }

    TEST_F(LexerRecoveryTest, PercentageBeforeNumber)
    {
        ExpectLexerRecovery("x = %1",
            {{E::InvalidCharacter, 1, 5}},
            {
                {TokenType::Identifier, "x", 1, 1},
                {TokenType::Assign, "=", 1, 3},
                {TokenType::Number, "1", 1, 6},
                {TokenType::EndOfFile, "", 1, 7}
            });
    }

    TEST_F(LexerRecoveryTest, UnterminatedDecimal1)
    {
        ExpectLexerRecovery("let a = 1.",
            {{E::UnterminatedDecimal, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::Number, "1.", 1, 9},
                {TokenType::EndOfFile, "", 1, 11}
            });
    }

    TEST_F(LexerRecoveryTest, UnterminatedDecimal2)
    {
        ExpectLexerRecovery("let a = 1_230.",
            {{E::UnterminatedDecimal, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::Number, "1_230.", 1, 9},
                {TokenType::EndOfFile, "", 1, 15}
            });
    }

    TEST_F(LexerRecoveryTest, UnterminatedNumberAfterSeparator)
    {
        ExpectLexerRecovery("let a = 1_",
            {{E::TrailingSeparatorInNumberLiteral, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::Number, "1_", 1, 9},
                {TokenType::EndOfFile, "", 1, 11}
            });
    }

    TEST_F(LexerRecoveryTest, AtDoubleUnderscoreInvalid)
    {
        ExpectLexerRecovery("let a = 1__000",
            {{E::TrailingSeparatorInNumberLiteral, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::Number, "1_", 1, 9},
                {TokenType::Identifier, "_000", 1, 11},
                {TokenType::EndOfFile, "", 1, 15}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedString1)
    {
        ExpectLexerRecovery("let a = \"hello",
            {{E::UnclosedString, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::String, "\"hello", 1, 9},
                {TokenType::EndOfFile, "", 1, 15}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedString2)
    {
        ExpectLexerRecovery("let a = \"hello \n \n",
            {{E::UnclosedString, 1, 9}},
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::String, "\"hello ", 1, 9},
                {TokenType::EndOfFile, "", 3, 1}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedString3)
    {
        ExpectLexerRecovery("\"hello",
            {{E::UnclosedString, 1, 1}},
            {
                {TokenType::String, "\"hello", 1, 1},
                {TokenType::EndOfFile, "", 1, 7}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedStringImport)
    {
        ExpectLexerRecovery("import \"file/path",
            {{E::UnclosedString, 1, 8}},
            {
                {TokenType::Import, "import", 1, 1},
                {TokenType::String, "\"file/path", 1, 8},
                {TokenType::EndOfFile, "", 1, 18}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedDocString1)
    {
        ExpectLexerRecovery(R"(func test() -> scalar { """"" return 1 })",
            {{E::UnclosedString, 1, 25}},
            {
                {TokenType::Func, "func", 1, 1},
                {TokenType::Identifier, "test", 1, 6},
                {TokenType::LeftParen, "(", 1, 10},
                {TokenType::RightParen, ")", 1, 11},
                {TokenType::Arrow, "->", 1, 13},
                {TokenType::Identifier, "scalar", 1, 16},
                {TokenType::LeftBrace, "{", 1, 23},
                {TokenType::DocString, R"(""""" return 1 })", 1, 25},
                {TokenType::EndOfFile, "", 1, 41}
            });
    }

    TEST_F(LexerRecoveryTest, UnclosedDocString2)
    {
        ExpectLexerRecovery(R"(""""" return 1 })",
            {{E::UnclosedString, 1, 1}},
            {
                {TokenType::DocString, R"(""""" return 1 })", 1, 1},
                {TokenType::EndOfFile, "", 1, 17}
            });
    }

    TEST_F(LexerRecoveryTest, MultipleInvalidCharacters)
    {
        ExpectLexerRecovery("let a = $\nlet b = ~\nlet c = \\",
            {
                {E::InvalidCharacter, 1, 9},
                {E::InvalidCharacter, 2, 9},
                {E::InvalidCharacter, 3, 9}
            },
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "a", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::Let, "let", 2, 1},
                {TokenType::Identifier, "b", 2, 5},
                {TokenType::Assign, "=", 2, 7},
                {TokenType::Let, "let", 3, 1},
                {TokenType::Identifier, "c", 3, 5},
                {TokenType::Assign, "=", 3, 7},
                {TokenType::EndOfFile, "", 3, 10}
            });
    }

    TEST_F(LexerRecoveryTest, MixedTokenErrors)
    {
        ExpectLexerRecovery("let w = 12.\nlet x = .5\nlet y = \"unclosed string spanning to EOF",
            {
                {E::UnterminatedDecimal, 1, 9},
                {E::UnclosedString, 3, 9}
            },
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "w", 1, 5},
                {TokenType::Assign, "=", 1, 7},
                {TokenType::Number, "12.", 1, 9},
                {TokenType::Let, "let", 2, 1},
                {TokenType::Identifier, "x", 2, 5},
                {TokenType::Assign, "=", 2, 7},
                {TokenType::Number, ".5", 2, 9},
                {TokenType::Let, "let", 3, 1},
                {TokenType::Identifier, "y", 3, 5},
                {TokenType::Assign, "=", 3, 7},
                {TokenType::String, "\"unclosed string spanning to EOF", 3, 9},
                {TokenType::EndOfFile, "", 3, 41}
            });
    }

    TEST_F(LexerRecoveryTest, SuccessiveInvalidCharacters)
    {
        ExpectLexerRecovery("$$$\n~~~",
            {
                {E::InvalidCharacter, 1, 1},
                {E::InvalidCharacter, 1, 2},
                {E::InvalidCharacter, 1, 3},
                {E::InvalidCharacter, 2, 1},
                {E::InvalidCharacter, 2, 2},
                {E::InvalidCharacter, 2, 3}
            },
            {
                {TokenType::EndOfFile, "", 2, 4}
            });
    }

    TEST_F(LexerRecoveryTest, InvalidUnderscoresInNumbers)
    {
        ExpectLexerRecovery("let num1 = 12__3\nlet num2 = 45._\nlet num3 = 100_",
            {
                {E::TrailingSeparatorInNumberLiteral, 1, 12},
                {E::UnterminatedDecimal, 2, 12},
                {E::TrailingSeparatorInNumberLiteral, 3, 12}
            },
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "num1", 1, 5},
                {TokenType::Assign, "=", 1, 10},
                {TokenType::Number, "12_", 1, 12},
                {TokenType::Identifier, "_3", 1, 15},
                {TokenType::Let, "let", 2, 1},
                {TokenType::Identifier, "num2", 2, 5},
                {TokenType::Assign, "=", 2, 10},
                {TokenType::Number, "45.", 2, 12},
                {TokenType::Identifier, "_", 2, 15},
                {TokenType::Let, "let", 3, 1},
                {TokenType::Identifier, "num3", 3, 5},
                {TokenType::Assign, "=", 3, 10},
                {TokenType::Number, "100_", 3, 12},
                {TokenType::EndOfFile, "", 3, 16}
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
                {E::InvalidCharacter, 4, 16},
                {E::UnterminatedDecimal, 5, 15},
                {E::InvalidCharacter, 6, 16},
                {E::UnclosedString, 8, 16}
            },
            {
                {TokenType::Let, "let", 3, 1},
                {TokenType::Identifier, "value", 3, 5},
                {TokenType::Assign, "=", 3, 11},
                {TokenType::Number, "100", 3, 13},
                {TokenType::Let, "let", 4, 1},
                {TokenType::Identifier, "invalid1", 4, 5},
                {TokenType::Assign, "=", 4, 14},
                {TokenType::Let, "let", 5, 1},
                {TokenType::Identifier, "partial", 5, 5},
                {TokenType::Assign, "=", 5, 13},
                {TokenType::Number, "0.", 5, 15},
                {TokenType::Let, "let", 6, 1},
                {TokenType::Identifier, "invalid2", 6, 5},
                {TokenType::Assign, "=", 6, 14},
                {TokenType::Let, "let", 8, 1},
                {TokenType::Identifier, "unclosed", 8, 5},
                {TokenType::Assign, "=", 8, 14},
                {TokenType::String, "\"started", 8, 16},
                {TokenType::EndOfFile, "", 9, 1}
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
                {E::UnclosedString, 1, 11}
            },
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "doc", 1, 5},
                {TokenType::Assign, "=", 1, 9},
                {TokenType::DocString, "\"\"\"This is a docstring\nthat spans multiple lines\nbut never closes properly...", 1, 11},
                {TokenType::EndOfFile, "", 3, 30}
            });
    }

    TEST_F(LexerRecoveryTest, HiddenLexicalErrorsInMath)
    {
        ExpectLexerRecovery("let result = 10 + .5 * 100_ - \"unclosed",
            {
                {E::TrailingSeparatorInNumberLiteral, 1, 24},
                {E::UnclosedString, 1, 31}
            },
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "result", 1, 5},
                {TokenType::Assign, "=", 1, 12},
                {TokenType::Number, "10", 1, 14},
                {TokenType::Plus, "+", 1, 17},
                {TokenType::Number, ".5", 1, 19},
                {TokenType::Star, "*", 1, 22},
                {TokenType::Number, "100_", 1, 24},
                {TokenType::Minus, "-", 1, 29},
                {TokenType::String, "\"unclosed", 1, 31},
                {TokenType::EndOfFile, "", 1, 40}
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
                {E::InvalidCharacter, 2, 12},
                {E::InvalidCharacter, 4, 12}
            },
            {
                {TokenType::Let, "let", 1, 1},
                {TokenType::Identifier, "valid1", 1, 5},
                {TokenType::Assign, "=", 1, 12},
                {TokenType::Number, "100", 1, 14},
                {TokenType::Let, "let", 2, 1},
                {TokenType::Identifier, "bad1", 2, 5},
                {TokenType::Assign, "=", 2, 10},
                {TokenType::Let, "let", 3, 1},
                {TokenType::Identifier, "valid2", 3, 5},
                {TokenType::Assign, "=", 3, 12},
                {TokenType::String, "\"test\"", 3, 14},
                {TokenType::Let, "let", 4, 1},
                {TokenType::Identifier, "bad2", 4, 5},
                {TokenType::Assign, "=", 4, 10},
                {TokenType::Let, "let", 5, 1},
                {TokenType::Identifier, "valid3", 5, 5},
                {TokenType::Assign, "=", 5, 12},
                {TokenType::Number, ".99", 5, 14},
                {TokenType::Let, "let", 6, 1},
                {TokenType::Identifier, "valid4", 6, 5},
                {TokenType::Assign, "=", 6, 12},
                {TokenType::Number, "0.99", 6, 14},
                {TokenType::EndOfFile, "", 6, 18}
            });
    }

    TEST_F(LexerRecoveryTest, RapidFireCorruptions)
    {
        std::string source = "\"unclosed 1\n12.\n\"unclosed 2\n.9\n1_a";

        ExpectLexerRecovery(source,
            {
                {E::UnclosedString, 1, 1},
                {E::UnterminatedDecimal, 2, 1},
                {E::UnclosedString, 3, 1},
                {E::TrailingSeparatorInNumberLiteral, 5, 1}
            },
            {
                {TokenType::String, "\"unclosed 1", 1, 1},
                {TokenType::Number, "12.", 2, 1},
                {TokenType::String, "\"unclosed 2", 3, 1},
                {TokenType::Number, ".9", 4, 1},
                {TokenType::Number, "1_", 5, 1},
                {TokenType::Identifier, "a", 5, 3},
                {TokenType::EndOfFile, "", 5, 4}
            });
    }

    TEST_F(LexerRecoveryTest, WhitespaceAndIndentationTracking)
    {
        std::string source = "\n\n\t\tlet spaced =   $\n    let padded = .123\n    let carriage = 12.\n";

        ExpectLexerRecovery(source,
            {
                {E::InvalidCharacter, 3, 18},
                {E::UnterminatedDecimal, 5, 20}
            },
            {
                {TokenType::Let, "let", 3, 3},
                {TokenType::Identifier, "spaced", 3, 7},
                {TokenType::Assign, "=", 3, 14},
                {TokenType::Let, "let", 4, 5},
                {TokenType::Identifier, "padded", 4, 9},
                {TokenType::Assign, "=", 4, 16},
                {TokenType::Number, ".123", 4, 18},
                {TokenType::Let, "let", 5, 5},
                {TokenType::Identifier, "carriage", 5, 9},
                {TokenType::Assign, "=", 5, 18},
                {TokenType::Number, "12.", 5, 20},
                {TokenType::EndOfFile, "", 6, 1}
            });
    }

    TEST_F(LexerRecoveryTest, PureGarbageFile)
    {
        std::string source = "?\\`\n~|\n$$$";

        ExpectLexerRecovery(source,
            {
                {E::InvalidCharacter, 1, 1},
                {E::InvalidCharacter, 1, 2},
                {E::InvalidCharacter, 1, 3},
                {E::InvalidCharacter, 2, 1},
                {E::InvalidCharacter, 2, 2},
                {E::InvalidCharacter, 3, 1},
                {E::InvalidCharacter, 3, 2},
                {E::InvalidCharacter, 3, 3}
            },
            {
                {TokenType::EndOfFile, "", 3, 4}
            });
    }
}
