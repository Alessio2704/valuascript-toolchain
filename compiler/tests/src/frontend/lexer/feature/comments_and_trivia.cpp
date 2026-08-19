#include <gtest/gtest.h>
#include "../lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerCommentTest : public LexerTestBase
    {
    };

    TEST_F(LexerCommentTest, StandaloneSingleLineComment)
    {
        auto ctx = std::make_shared<CompilerContext>();
        std::string source = "// this is a comment\nlet x = 1";

        auto tokens = tokenize_code(source, true, ctx);

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[0].start_offset, 21);
        EXPECT_EQ(tokens[0].line, 2);

        const auto& comments = ctx->get_comments("test.vs");
        ASSERT_EQ(comments.size(), 1);
        EXPECT_EQ(comments[0].text, "// this is a comment");
        EXPECT_EQ(comments[0].line, 1);
        EXPECT_EQ(comments[0].column, 1);
        EXPECT_EQ(comments[0].start_offset, 0);
        EXPECT_EQ(comments[0].length, 20);
        EXPECT_EQ(comments[0].span.line_start, 1);
        EXPECT_EQ(comments[0].span.column_start, 1);
        EXPECT_EQ(comments[0].span.line_end, 1);
        EXPECT_EQ(comments[0].span.column_end, 21);
        EXPECT_EQ(comments[0].span.start_offset, 0);
        EXPECT_EQ(comments[0].span.length, 20);
    }

    TEST_F(LexerCommentTest, InlineCommentAndLinterDirectives)
    {
        auto ctx = std::make_shared<CompilerContext>();
        std::string source = "let x = 1 // vs-lint:disable-line";

        auto tokens = tokenize_code(source, true, ctx);

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[0].type, TokenType::Let);
        EXPECT_EQ(tokens[1].type, TokenType::Identifier);
        EXPECT_EQ(tokens[2].type, TokenType::Assign);
        EXPECT_EQ(tokens[3].type, TokenType::Number);
        EXPECT_EQ(tokens[4].type, TokenType::EndOfFile);

        const auto& comments = ctx->get_comments("test.vs");
        ASSERT_EQ(comments.size(), 1);
        EXPECT_EQ(comments[0].text, "// vs-lint:disable-line");
        EXPECT_EQ(comments[0].line, 1);
        EXPECT_EQ(comments[0].column, 11);
        EXPECT_EQ(comments[0].start_offset, 10);
        EXPECT_EQ(comments[0].length, 23);
        EXPECT_EQ(comments[0].span.line_start, 1);
        EXPECT_EQ(comments[0].span.column_start, 11);
    }

    TEST_F(LexerCommentTest, TrailingCommentWithoutTrailingNewline)
    {
        auto ctx = std::make_shared<CompilerContext>();
        std::string source = "let x = 1\n// end of file comment";

        auto tokens = tokenize_code(source, true, ctx);

        ASSERT_EQ(tokens.size(), 5);
        EXPECT_EQ(tokens[4].type, TokenType::EndOfFile);

        const auto& comments = ctx->get_comments("test.vs");
        ASSERT_EQ(comments.size(), 1);
        EXPECT_EQ(comments[0].text, "// end of file comment");
        EXPECT_EQ(comments[0].line, 2);
        EXPECT_EQ(comments[0].column, 1);
        EXPECT_EQ(comments[0].start_offset, 10);
        EXPECT_EQ(comments[0].length, 22);
    }

    TEST_F(LexerCommentTest, MultipleConsecutiveComments)
    {
        auto ctx = std::make_shared<CompilerContext>();
        std::string source = "// first line\n// second line\n// third line\n";

        auto tokens = tokenize_code(source, true, ctx);

        ASSERT_EQ(tokens.size(), 1);
        EXPECT_EQ(tokens[0].type, TokenType::EndOfFile);

        const auto& comments = ctx->get_comments("test.vs");
        ASSERT_EQ(comments.size(), 3);

        EXPECT_EQ(comments[0].text, "// first line");
        EXPECT_EQ(comments[0].line, 1);
        EXPECT_EQ(comments[0].start_offset, 0);
        EXPECT_EQ(comments[0].length, 13);

        EXPECT_EQ(comments[1].text, "// second line");
        EXPECT_EQ(comments[1].line, 2);
        EXPECT_EQ(comments[1].start_offset, 14);
        EXPECT_EQ(comments[1].length, 14);

        EXPECT_EQ(comments[2].text, "// third line");
        EXPECT_EQ(comments[2].line, 3);
        EXPECT_EQ(comments[2].start_offset, 29);
        EXPECT_EQ(comments[2].length, 13);
    }
}
