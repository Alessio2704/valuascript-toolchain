#include <gtest/gtest.h>
#include "stages/frontend/lexer/lexer_stage.h"
#include "token/token.h"
#include "errors/valuascript_exception.h"
#include "frontend/lexer/lexer_tests_utils.h"
#include <string>
#include <vector>

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    TEST(LexerStageTest, TokenizesValuaScriptCorrectly) {
        LexerStage lexer_stage;

        std::string source_code = "let a = 1_000.50\nfunc main() { return a } // A comment\n@directive \"math\"";
        std::vector<CompilerStageArtifact> history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, source_code}
        };

        auto tokens = tokenize_code(source_code);
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 16);

        for (const auto &a: tokens) {
            std::cout << a.lexeme << std::endl;
        }

        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[1].lexeme, "a");
        EXPECT_EQ(tokens[2].type, TokenType::Assign);
        EXPECT_EQ(tokens[3].type, TokenType::Number);
        EXPECT_EQ(tokens[3].lexeme, "1_000.50");

        EXPECT_EQ(tokens[12].type, TokenType::At);
        EXPECT_EQ(tokens[13].type, TokenType::Identifier);
        EXPECT_EQ(tokens[14].type, TokenType::String);
    }

    TEST(LexerStageTest, TokenizesSingleCharacterOperators) {
        auto tokens = tokenize_code("()[]{},:@ +-*/^.#");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 17);
        EXPECT_EQ(tokens[0].type, TokenType::LeftParen);
        EXPECT_EQ(tokens[1].type, TokenType::RightParen);
        EXPECT_EQ(tokens[2].type, TokenType::LeftBracket);
        EXPECT_EQ(tokens[3].type, TokenType::RightBracket);
        EXPECT_EQ(tokens[4].type, TokenType::LeftBrace);
        EXPECT_EQ(tokens[5].type, TokenType::RightBrace);
        EXPECT_EQ(tokens[6].type, TokenType::Comma);
        EXPECT_EQ(tokens[7].type, TokenType::Colon);
        EXPECT_EQ(tokens[8].type, TokenType::At);
        EXPECT_EQ(tokens[9].type, TokenType::Plus);
        EXPECT_EQ(tokens[10].type, TokenType::Minus);
        EXPECT_EQ(tokens[11].type, TokenType::Star);
        EXPECT_EQ(tokens[12].type, TokenType::Slash);
        EXPECT_EQ(tokens[13].type, TokenType::Caret);
        EXPECT_EQ(tokens[14].type, TokenType::Dot);
        EXPECT_EQ(tokens[15].type, TokenType::Hash);
    }

    TEST(LexerStageTest, TokenizesMultiCharacterOperators) {
        const auto tokens = tokenize_code("= == != > >= < <= ->");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 9);
        EXPECT_EQ(tokens[0].type, TokenType::Assign);
        EXPECT_EQ(tokens[1].type, TokenType::Equals);
        EXPECT_EQ(tokens[2].type, TokenType::NotEquals);
        EXPECT_EQ(tokens[3].type, TokenType::Greater);
        EXPECT_EQ(tokens[4].type, TokenType::GreaterEqual);
        EXPECT_EQ(tokens[5].type, TokenType::Less);
        EXPECT_EQ(tokens[6].type, TokenType::LessEqual);
        EXPECT_EQ(tokens[7].type, TokenType::Arrow);
    }

    TEST(LexerStageTest, TokenizesKeywords) {
        auto tokens = tokenize_code(
            "let if then else true false and or not func struct return import enum switch case default mod var self typealias");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 22);
        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[1].type, TokenType::If);
        EXPECT_EQ(tokens[2].type, TokenType::Then);
        EXPECT_EQ(tokens[3].type, TokenType::Else);
        EXPECT_EQ(tokens[4].type, TokenType::True);
        EXPECT_EQ(tokens[5].type, TokenType::False);
        EXPECT_EQ(tokens[6].type, TokenType::And);
        EXPECT_EQ(tokens[7].type, TokenType::Or);
        EXPECT_EQ(tokens[8].type, TokenType::Not);
        EXPECT_EQ(tokens[9].type, TokenType::Func);
        EXPECT_EQ(tokens[10].type, TokenType::Struct);
        EXPECT_EQ(tokens[11].type, TokenType::Return);
        EXPECT_EQ(tokens[12].type, TokenType::Import);
        EXPECT_EQ(tokens[13].type, TokenType::Enum);
        EXPECT_EQ(tokens[14].type, TokenType::Switch);
        EXPECT_EQ(tokens[15].type, TokenType::Case);
        EXPECT_EQ(tokens[16].type, TokenType::Default);
        EXPECT_EQ(tokens[17].type, TokenType::Mod);
        EXPECT_EQ(tokens[18].type, TokenType::Var);
        EXPECT_EQ(tokens[19].type, TokenType::Self);
        EXPECT_EQ(tokens[20].type, TokenType::Typealias);
    }

    TEST(LexerStageTest, DistinguishesKeywordsFromIdentifiers) {
        const auto tokens = tokenize_code("let letter = format");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[1].lexeme, "letter");
        EXPECT_EQ(tokens[2].type, TokenType::Assign);
        EXPECT_EQ(tokens[3].type, TokenType::Identifier);
        EXPECT_EQ(tokens[3].lexeme, "format");
    }

    TEST(LexerStageTest, DistinguishesKeywordsInsideStrings) {
        const auto tokens = tokenize_code("let letter = \"bool\"");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[1].lexeme, "letter");
        EXPECT_EQ(tokens[2].type, TokenType::Assign);
        EXPECT_EQ(tokens[3].type, TokenType::String);
        EXPECT_EQ(tokens[3].lexeme, "\"bool\"");
    }

    TEST(LexerStageTest, TokenizesNumbersWithSeparators) {
        const auto tokens = tokenize_code("123 45.67 1_000_000 0.000_1");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[0].lexeme, "123");
        EXPECT_EQ(tokens[1].lexeme, "45.67");
        EXPECT_EQ(tokens[2].lexeme, "1_000_000");
        EXPECT_EQ(tokens[3].lexeme, "0.000_1");

        for (int i = 0; i < 4; ++i) {
            EXPECT_EQ(tokens[i].type, TokenType::Number);
        }
    }

    TEST(LexerStageTest, TokenizesNumbersWithPercentage) {
        const auto tokens = tokenize_code("4% 4.54% 0.54%");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 4);
        EXPECT_EQ(tokens[0].lexeme, "4%");
        EXPECT_EQ(tokens[1].lexeme, "4.54%");
        EXPECT_EQ(tokens[2].lexeme, "0.54%");

        for (int i = 0; i < 3; ++i) {
            EXPECT_EQ(tokens[i].type, TokenType::PercentageLiteral);
        }
    }

    TEST(LexerStageTest, TokenizesStringsAndDocStrings) {
        const auto tokens = tokenize_code("\"hello world\" \"\"\"This is a\nmultiline docstring\"\"\"");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 3);
        EXPECT_EQ(tokens[0].type, TokenType::String);
        EXPECT_EQ(tokens[0].lexeme, "\"hello world\"");

        EXPECT_EQ(tokens[1].type, TokenType::DocString);
        EXPECT_EQ(tokens[1].lexeme, "\"\"\"This is a\nmultiline docstring\"\"\"");
    }

    TEST(LexerStageTest, IgnoresWhitespaceAndComments) {
        const std::string code =
                "let a = 5\n"
                "// This is a comment\n"
                "    \t return a";

        const auto tokens = tokenize_code(code);
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 7);
        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[2].type, TokenType::Assign);
        EXPECT_EQ(tokens[3].type, TokenType::Number);
        EXPECT_EQ(tokens[4].type, TokenType::Return);
        EXPECT_EQ(tokens[5].type, TokenType::Identifier);
    }

    TEST(LexerStageTest, TracksLineAndColumnAccurately) {
        const std::string code =
                "let a\n" // Line 1: 'let' at col 1, 'a' at col 5
                "  return\n"; // Line 2: 'return' at col 3

        const auto tokens = tokenize_code(code);
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        EXPECT_EQ(tokens[0].line, 1);
        EXPECT_EQ(tokens[0].column, 1);

        EXPECT_EQ(tokens[1].line, 1);
        EXPECT_EQ(tokens[1].column, 5);

        EXPECT_EQ(tokens[2].line, 2);
        EXPECT_EQ(tokens[2].column, 3);
    }

    TEST(LexerStageTest, HandlesEmptyFile) {
        const auto tokens = tokenize_code("");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 1);
        EXPECT_EQ(tokens[0].type, TokenType::EndOfFile);
        EXPECT_EQ(tokens[0].line, 1);
        EXPECT_EQ(tokens[0].column, 1);
    }

    TEST(LexerStageTest, TokenizesComplexIdentifiers) {
        const auto tokens = tokenize_code("let _privateVar123 = 0");

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[1].lexeme, "_privateVar123");
    }

    TEST(LexerStageTest, DocStringContainsRegularQuotes) {
        const auto tokens = tokenize_code(R"("""This docstring has "quotes" inside""")");

        ASSERT_EQ(tokens.size(), 2);
        EXPECT_EQ(tokens[0].type, TokenType::DocString);

        EXPECT_EQ(tokens[0].lexeme, "\"\"\"This docstring has \"quotes\" inside\"\"\"");
    }

    TEST(LexerStageTest, HandlesConsecutiveOperatorsWithoutSpaces) {
        const auto tokens = tokenize_code("if(a==-1)");

        ASSERT_EQ(tokens.size(), 8);
        EXPECT_EQ(tokens[3].type, TokenType::Equals);
        EXPECT_EQ(tokens[4].type, TokenType::Minus);
        EXPECT_EQ(tokens[5].type, TokenType::Number);
    }

    TEST(LexerStageTest, CatchesInvalidCharacterWithDetail) {
        try {
            tokenize_code("let a = $;");
            FAIL() << "Lexer should have caught the invalid '$' character.";
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Lexical);
            EXPECT_EQ(e.get_code(), ValuascriptErrorCode::InvalidCharacter);

            const std::string expected_msg = "Syntax Error: Invalid character '$' found.";

            const std::string actual_full_msg = e.what();
            EXPECT_TRUE(actual_full_msg.find(expected_msg) != std::string::npos)
            << "Message did not match. Actual: " << actual_full_msg;
        }
    }

    TEST(LexerStageTest, CatchesUnclosedStringWithDetail) {
        try {
            tokenize_code("let greeting = \"Hello World");
            FAIL() << "Lexer should have caught the unclosed string.";
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Lexical);
            EXPECT_EQ(e.get_code(), ValuascriptErrorCode::UnclosedString);

            const std::string expected_msg = "Syntax Error: Unclosed string literal.";
            const std::string actual_full_msg = e.what();

            EXPECT_TRUE(actual_full_msg.find(expected_msg) != std::string::npos)
            << "Message did not match. Actual: " << actual_full_msg;
        }
    }

    TEST(LexerStageTest, CatchesUnclosedDocStringWithDetail) {
        try {
            tokenize_code(R"(func main() { """This docstring never ends })");
            FAIL() << "Lexer should have caught the unclosed docstring.";
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Lexical);
            EXPECT_EQ(e.get_code(), ValuascriptErrorCode::UnclosedString);

            const std::string expected_msg = "Syntax Error: Unclosed string literal.";
            const std::string actual_full_msg = e.what();

            EXPECT_TRUE(actual_full_msg.find(expected_msg) != std::string::npos)
            << "Message did not match. Actual: " << actual_full_msg;
        }
    }

    TEST(LexerStageTest, TokenizesFunctionWithExplicitTypes) {
        auto tokens = tokenize_code("func test(a: scalar, b: vector) { return a + b }");
        ASSERT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);

        ASSERT_EQ(tokens.size(), 18);

        EXPECT_EQ(tokens[0].type, TokenType::Func);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[2].type, TokenType::LeftParen);

        EXPECT_EQ(tokens[3].type, TokenType::Identifier);
        EXPECT_EQ(tokens[4].type, TokenType::Colon);
        EXPECT_EQ(tokens[5].type, TokenType::Identifier);

        EXPECT_EQ(tokens[6].type, TokenType::Comma);

        EXPECT_EQ(tokens[7].type, TokenType::Identifier);
        EXPECT_EQ(tokens[8].type, TokenType::Colon);
        EXPECT_EQ(tokens[9].type, TokenType::Identifier);

        EXPECT_EQ(tokens[10].type, TokenType::RightParen);

        EXPECT_EQ(tokens[11].type, TokenType::LeftBrace);
        EXPECT_EQ(tokens[12].type, TokenType::Return);
        EXPECT_EQ(tokens[13].type, TokenType::Identifier);
        EXPECT_EQ(tokens[14].type, TokenType::Plus);
        EXPECT_EQ(tokens[15].type, TokenType::Identifier);
        EXPECT_EQ(tokens[16].type, TokenType::RightBrace);

        EXPECT_EQ(tokens[17].type, TokenType::EndOfFile);
    }
}
