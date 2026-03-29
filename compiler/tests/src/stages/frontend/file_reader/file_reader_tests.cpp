#include <gtest/gtest.h>
#include "stages/frontend/file_reader/file_reader_stage.h"
#include <fstream>

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class FileReaderBaseTest : public testing::Test {
    protected:
        std::pair<CompilerStageArtifactCode, std::string> static read_file(const std::string &test_file_path) {
            FileReaderStage reader;
            auto context = std::make_shared<CompilerContext>();
            std::vector<CompilerStageArtifact> history = {
                {CompilerStageArtifactCode::FilePath, test_file_path}
            };

            auto result_artifact = reader.run(*context, history);
            auto actual_content = std::any_cast<std::string>(result_artifact.data);
            return std::make_pair(result_artifact.code, actual_content);
        }
    };

    TEST_F(FileReaderBaseTest, SuccessfullyReadsFile) {
        std::string test_file_path = "dummy_test_file.vs";
        std::string expected_content = "let a = 10\nfunc main() { return a }";

        std::ofstream out_file(test_file_path);
        out_file << expected_content;
        out_file.close();

        FileReaderStage reader;
        std::vector<CompilerStageArtifact> history = {
            {CompilerStageArtifactCode::FilePath, test_file_path}
        };

        auto [code, data] = read_file(test_file_path);

        EXPECT_EQ(code, CompilerStageArtifactCode::SourceCode);
        EXPECT_EQ(data, expected_content);

        std::remove(test_file_path.c_str());
    }

    TEST_F(FileReaderBaseTest, ThrowsOnMissingFile) {
        try {
            read_file("non_existent_file.vs");
        } catch (const ValuaScriptException &err) {
            EXPECT_EQ(err.get_code(), ValuascriptErrorCode::FileNotFound);
            EXPECT_EQ(err.get_category(), ValuascriptErrorCategory::File);
        } catch (...) {
            FAIL() << "Expected ValuaScriptException, but a different exception was thrown";
        }
    }
}
