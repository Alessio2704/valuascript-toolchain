#include <gtest/gtest.h>
#include "file_reader_test_base.h"

namespace valuascript::compiler::test
{
    class FileReaderHappyPathTest : public FileReaderTestBase
    {
    };

    TEST_F(FileReaderHappyPathTest, SuccessfullyReadsStandardFile)
    {
        std::string expected_content = "let a = 10\nfunc main() { return a }";
        auto file_path = CreateTestFile("standard_file.vs", expected_content);

        ExpectFileReadSuccess(file_path.string(), expected_content);
    }

    TEST_F(FileReaderHappyPathTest, SuccessfullyReadsEmptyFile)
    {
        auto file_path = CreateTestFile("empty_file.vs", "");

        ExpectFileReadSuccess(file_path.string(), "");
    }

    TEST_F(FileReaderHappyPathTest, NormalizesCRLFLineEndings)
    {
        std::string raw_crlf_content = "let a = 10\r\nfunc main() {\r\n    return a\r\n}";
        std::string expected_normalized = "let a = 10\nfunc main() {\n    return a\n}";

        auto file_path = CreateTestFile("crlf_file.vs", raw_crlf_content);

        ExpectFileReadSuccess(file_path.string(), expected_normalized);
    }

    TEST_F(FileReaderHappyPathTest, UpdatesSourceRegistry)
    {
        std::string expected_content = "let x = 42";
        auto file_path = CreateTestFile("registry_test.vs", expected_content);
        auto context = std::make_shared<CompilerContext>();

        ExpectFileReadSuccess(file_path.string(), expected_content, context);

        std::string canonical_path = std::filesystem::weakly_canonical(file_path).string();
        auto source_opt = context->source_manager.get_source(canonical_path);
        ASSERT_TRUE(source_opt.has_value());
        EXPECT_EQ(source_opt.value(), expected_content);
    }

    TEST_F(FileReaderHappyPathTest, HandlesUnicodeUTF8Content)
    {
        std::string utf8_content = "// 🚀 ValuaScript test with UTF-8: ñ, é, 𝛌, 𝝅\nlet π = 3.14159";
        auto file_path = CreateTestFile("utf8_file.vs", utf8_content);

        ExpectFileReadSuccess(file_path.string(), utf8_content);
    }
}
