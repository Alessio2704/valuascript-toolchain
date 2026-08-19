#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstCommentsAndOffsetsSpanTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstCommentsAndOffsetsSpanTest, ExactByteOffsetsOnBinaryAndAssignment)
    {
        std::string code = "let total = 10 + 20";
        auto ast = parse_code(code);
        size_t bin_offset = code.find("10 + 20");
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{.name = "total"}
                        .with_name_span(1, 5, 1, 10, 4, 5)
                        .with_span(1, 5, 1, 10, 4, 5)
                },
                IsBinary(TokenType::Plus,
                    IsNumber("10").with_span(1, 13, 1, 15, 12, 2),
                    IsNumber("20").with_span(1, 18, 1, 20, 17, 2)
                ).with_span(1, 13, 1, 20, bin_offset, 7)
                ).with_span(1, 1, 1, 20, 0, code.length())
            }
        });
    }

    TEST_F(AstCommentsAndOffsetsSpanTest, ProgramCommentsPreservationAndOffsetTracking)
    {
        std::string code =
            "// Header comment\n"
            "let x = 42 // vs-lint:disable-line\n"
            "// Footer comment";
        auto ast = parse_code(code);

        ASSERT_EQ(ast->comments.size(), 3);
        EXPECT_EQ(ast->comments[0].text, "// Header comment");
        EXPECT_EQ(ast->comments[0].start_offset, 0);
        EXPECT_EQ(ast->comments[0].length, 17);

        EXPECT_EQ(ast->comments[1].text, "// vs-lint:disable-line");
        size_t c1_off = code.find("// vs-lint");
        EXPECT_EQ(ast->comments[1].start_offset, c1_off);

        EXPECT_EQ(ast->comments[2].text, "// Footer comment");
        size_t c2_off = code.find("// Footer");
        EXPECT_EQ(ast->comments[2].start_offset, c2_off);
    }
}
