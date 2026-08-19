#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    class LexerRecoveryStringLiteralsTest : public LexerTestBase
    {
    };

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedSingleLineStringIsolated)
    {
        ExpectLexerRecovery("\"hello",
            {{.code = E::UnclosedString, .line = 1, .column = 1, .start_offset = 0, .length = 6}},
            {
                {.type = TokenType::String, .lexeme = "\"hello", .line = 1, .column = 1, .start_offset = 0, .length = 6}
            });
    }

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedSingleLineStringWithNewlines)
    {
        ExpectLexerRecovery("\"hello \n \n",
            {{.code = E::UnclosedString, .line = 1, .column = 1, .start_offset = 0, .length = 7}},
            {
                {.type = TokenType::String, .lexeme = "\"hello ", .line = 1, .column = 1, .start_offset = 0, .length = 7}
            });
    }

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedStringInExpression)
    {
        ExpectLexerRecovery("a = \"hello",
            {{.code = E::UnclosedString, .line = 1, .column = 5, .start_offset = 4, .length = 6}},
            {
                {.type = TokenType::Identifier, .lexeme = "a",      .line = 1, .column = 1, .start_offset = 0, .length = 1},
                {.type = TokenType::Assign,     .lexeme = "=",      .line = 1, .column = 3, .start_offset = 2, .length = 1},
                {.type = TokenType::String,     .lexeme = "\"hello", .line = 1, .column = 5, .start_offset = 4, .length = 6}
            });
    }

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedStringInImport)
    {
        ExpectLexerRecovery("import \"file/path",
            {{.code = E::UnclosedString, .line = 1, .column = 8, .start_offset = 7, .length = 10}},
            {
                {.type = TokenType::Import, .lexeme = "import",      .line = 1, .column = 1, .start_offset = 0, .length = 6},
                {.type = TokenType::String, .lexeme = "\"file/path", .line = 1, .column = 8, .start_offset = 7, .length = 10}
            });
    }

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedDocStringIsolated)
    {
        ExpectLexerRecovery(R"(""""" return 1 })",
            {{.code = E::UnclosedString, .line = 1, .column = 1, .start_offset = 0, .length = 16}},
            {
                {.type = TokenType::DocString, .lexeme = R"(""""" return 1 })", .line = 1, .column = 1, .start_offset = 0, .length = 16}
            });
    }

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedDocStringInFunction)
    {
        ExpectLexerRecovery(R"(func test() -> scalar { """"" return 1 })",
            {{.code = E::UnclosedString, .line = 1, .column = 25, .start_offset = 24, .length = 16}},
            {
                {.type = TokenType::Func,       .lexeme = "func",               .line = 1, .column = 1,  .start_offset = 0,  .length = 4},
                {.type = TokenType::Identifier, .lexeme = "test",               .line = 1, .column = 6,  .start_offset = 5,  .length = 4},
                {.type = TokenType::LeftParen,  .lexeme = "(",                  .line = 1, .column = 10, .start_offset = 9,  .length = 1},
                {.type = TokenType::RightParen, .lexeme = ")",                  .line = 1, .column = 11, .start_offset = 10, .length = 1},
                {.type = TokenType::Arrow,      .lexeme = "->",                 .line = 1, .column = 13, .start_offset = 12, .length = 2},
                {.type = TokenType::Identifier, .lexeme = "scalar",             .line = 1, .column = 16, .start_offset = 15, .length = 6},
                {.type = TokenType::LeftBrace,  .lexeme = "{",                  .line = 1, .column = 23, .start_offset = 22, .length = 1},
                {.type = TokenType::DocString,  .lexeme = R"(""""" return 1 })", .line = 1, .column = 25, .start_offset = 24, .length = 16}
            });
    }

    TEST_F(LexerRecoveryStringLiteralsTest, UnclosedDocStringMultiline)
    {
        std::string source =
            "doc = \"\"\"This is a docstring\n"
            "that spans multiple lines\n"
            "but never closes properly...";

        ExpectLexerRecovery(source,
            {
                {.code = E::UnclosedString, .line = 1, .column = 7}
            },
            {
                {.type = TokenType::Identifier, .lexeme = "doc",                                                                                             .line = 1, .column = 1, .start_offset = 0, .length = 3},
                {.type = TokenType::Assign,     .lexeme = "=",                                                                                               .line = 1, .column = 5, .start_offset = 4, .length = 1},
                {.type = TokenType::DocString,  .lexeme = "\"\"\"This is a docstring\nthat spans multiple lines\nbut never closes properly...",               .line = 1, .column = 7, .start_offset = 6, .length = 77}
            });
    }
}
