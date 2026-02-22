#include <gtest/gtest.h>
#include "stages/file_reader/file_reader_stage.h"
#include <fstream>

using namespace valuascript::compiler;

TEST(FileReaderStageTest, SuccessfullyReadsFile) {
    std::string test_file_path = "dummy_test_file.vs";
    std::string expected_content = "let a = 10\nfunc main() { return a }";
    
    std::ofstream out_file(test_file_path);
    out_file << expected_content;
    out_file.close();

    FileReaderStage reader;
    std::vector<CompilerStageArtifact> history = {
        {CompilerStageArtifactCode::FilePath, test_file_path}
    };
    
    auto result_artifact = reader.run(history);
    auto actual_content = std::any_cast<std::string>(result_artifact.data);

    EXPECT_EQ(result_artifact.code, CompilerStageArtifactCode::SourceCode);
    EXPECT_EQ(actual_content, expected_content);

    std::remove(test_file_path.c_str());
}

TEST(FileReaderStageTest, ThrowsOnMissingFile) {
    FileReaderStage reader;
    std::vector<CompilerStageArtifact> history = {
        {CompilerStageArtifactCode::FilePath, std::string("non_existent_file.vs")}
    };
    
    EXPECT_THROW(reader.run(history), std::runtime_error);
}