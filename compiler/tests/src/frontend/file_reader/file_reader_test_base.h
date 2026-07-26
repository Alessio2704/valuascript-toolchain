#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "frontend/file_reader/file_reader_stage.h"
#include "frontend/file_reader/file_reader_error_code.h"
#include "core/compiler_context.h"
#include "core/valuascript_exception.h"
#include "utils/pid.h"

namespace valuascript::compiler::test
{
    class FileReaderTestBase : public testing::Test
    {
    protected:
        std::filesystem::path temp_dir;

        void SetUp() override
        {
            temp_dir = generate_test_workspace("vs_file_reader_test", reinterpret_cast<uintptr_t>(this));
        }

        void TearDown() override
        {
            cleanup_test_workspace(temp_dir);
        }

        std::filesystem::path CreateTestFile(const std::string& filename, const std::string& content)
        {
            std::filesystem::path file_path = temp_dir / filename;
            std::ofstream out_file(file_path, std::ios::binary);
            out_file << content;
            out_file.close();
            return file_path;
        }

        std::pair<CompilerStageArtifactCode, std::string> RunFileReader(
            const std::string& test_file_path,
            bool fail_fast = true,
            std::shared_ptr<CompilerContext> context = nullptr)
        {
            if (!context)
            {
                context = std::make_shared<CompilerContext>();
            }
            context->settings.fail_fast = fail_fast;

            FileReaderStage reader;
            std::vector<CompilerStageArtifact> history = {
                {CompilerStageArtifactCode::FilePath, test_file_path}
            };

            auto result_artifact = reader.run(*context, history);
            auto actual_content = extract_artifact_data<std::string>({result_artifact}, CompilerStageArtifactCode::SourceCode);
            return {result_artifact.code, actual_content};
        }

        void ExpectFileReadSuccess(const std::string& test_file_path,
                                   const std::string& expected_content,
                                   std::shared_ptr<CompilerContext> context = nullptr)
        {
            if (!context)
            {
                context = std::make_shared<CompilerContext>();
            }

            CompilerStageArtifactCode code;
            std::string content;
            ASSERT_NO_THROW({
                std::tie(code, content) = RunFileReader(test_file_path, true, context);
            }) << "FileReaderStage threw an unexpected exception for file: " << test_file_path;

            EXPECT_EQ(code, CompilerStageArtifactCode::SourceCode);
            EXPECT_EQ(content, expected_content);
        }

        void ExpectFileReaderError(const std::string& test_file_path, FileReaderErrorCode expected_error)
        {
            try
            {
                RunFileReader(test_file_path, true);
                FAIL() << "Expected FileReader error " << static_cast<int>(expected_error)
                       << " but reading succeeded for file: " << test_file_path;
            }
            catch (const ValuaScriptException& err)
            {
                EXPECT_EQ(err.get_category(), ValuascriptErrorCategory::File);
                EXPECT_TRUE(err.is_error(expected_error))
                    << "Error code mismatch for file: " << test_file_path << ". Actual error: " << err.what();
            }
            catch (...)
            {
                FAIL() << "FileReader threw an unexpected non-ValuaScriptException for file: " << test_file_path;
            }
        }
    };
}
