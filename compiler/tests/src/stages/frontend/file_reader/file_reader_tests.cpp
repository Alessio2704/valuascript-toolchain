#include <gtest/gtest.h>
#include <fstream>
#include "stages/frontend/file_reader/file_reader_stage.h"
#include "utils/pid.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class FileReaderBaseTest : public testing::Test {
    protected:
        std::filesystem::path temp_dir;

        void SetUp() override {
            temp_dir = generate_test_workspace("vs_base_test", reinterpret_cast<uintptr_t>(this));
        }

        void TearDown() override {
            cleanup_test_workspace(temp_dir);
        }

        std::pair<CompilerStageArtifactCode, std::string> read_file(const std::string &test_file_path) {
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
        std::string test_file_path = (temp_dir / "dummy_test_file.vs").string();
        std::string expected_content = "let a = 10\nfunc main() { return a }";

        std::ofstream out_file(test_file_path, std::ios::binary);
        out_file << expected_content;
        out_file.close();

        auto [code, data] = read_file(test_file_path);

        EXPECT_EQ(code, CompilerStageArtifactCode::SourceCode);
        EXPECT_EQ(data, expected_content);
    }

    TEST_F(FileReaderBaseTest, ThrowsOnMissingFile) {
        std::string ghost_path = (temp_dir / "non_existent_file.vs").string();
        try {
            read_file(ghost_path);
            FAIL() << "Expected ValuaScriptException was not thrown";
        } catch (const ValuaScriptException &err) {
            EXPECT_EQ(err.get_code(), ValuascriptErrorCode::FileNotFound);
            EXPECT_EQ(err.get_category(), ValuascriptErrorCategory::File);
        } catch (...) {
            FAIL() << "Expected ValuaScriptException, but a different exception was thrown";
        }
    }
}
