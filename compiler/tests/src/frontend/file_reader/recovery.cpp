#include <gtest/gtest.h>
#include "file_reader_test_base.h"

namespace valuascript::compiler::test
{
    using E = FileReaderErrorCode;

    class FileReaderSadPathTest : public FileReaderTestBase
    {
    };

    TEST_F(FileReaderSadPathTest, ThrowsOnMissingFile)
    {
        std::string ghost_path = (temp_dir / "non_existent_file.vs").string();
        ExpectFileReaderError(ghost_path, E::FileNotFound);
    }

    TEST_F(FileReaderSadPathTest, ThrowsOnInvalidPath)
    {
        std::string invalid_path = (temp_dir / "non_existent_dir" / "ghost_file.vs").string();
        ExpectFileReaderError(invalid_path, E::FileNotFound);
    }

    TEST_F(FileReaderSadPathTest, ReportsErrorDiagnosticsInNonFailFast)
    {
        std::string ghost_path = (temp_dir / "missing_file.vs").string();
        auto context = std::make_shared<CompilerContext>();

        ASSERT_NO_THROW({
            RunFileReader(ghost_path, /*fail_fast=*/false, context);
        });

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_EQ(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::File);
        EXPECT_TRUE(errors[0].is_error(E::FileNotFound));
    }
}
