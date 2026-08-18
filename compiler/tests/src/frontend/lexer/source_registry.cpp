#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "lexer_test_base.h"

namespace valuascript::compiler::test
{
    class LexerSourceRegistryTest : public LexerTestBase
    {
    };

    TEST_F(LexerSourceRegistryTest, UpdatesSourceRegistryOnTokenize)
    {
        std::string code = "let x = 42";
        auto context = std::make_shared<CompilerContext>();

        tokenize_code(code, true, context);

        auto source_opt = context->source_manager.get_source("test.vs");
        ASSERT_TRUE(source_opt.has_value());
        EXPECT_EQ(source_opt.value(), code);
    }
}
