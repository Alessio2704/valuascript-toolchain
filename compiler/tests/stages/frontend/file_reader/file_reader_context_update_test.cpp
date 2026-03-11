#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include "stages/frontend/file_reader/file_reader_stage.h"
#include "../../../../include/compiler_context/compiler_context.h"

using namespace valuascript::compiler;

class FileReaderStageTest : public ::testing::Test {
protected:
    std::string temp_file_path = "test_registry_source.vs";
    std::string expected_source = "let x = 100;\nfunc test() { return x; }";

    void SetUp() override {
        std::ofstream out(temp_file_path);
        out << expected_source;
        out.close();
    }

    void TearDown() override {
        std::remove(temp_file_path.c_str());
    }
};

TEST_F(FileReaderStageTest, PopulatesSourceRegistryInContext) {
    auto context = std::make_shared<CompilerContext>();
    FileReaderStage reader;

    std::vector<CompilerStageArtifact> initial_artifacts = {
        {CompilerStageArtifactCode::FilePath, temp_file_path}
    };

    auto result_artifact = reader.run(context, initial_artifacts);

    EXPECT_EQ(result_artifact.code, CompilerStageArtifactCode::SourceCode);
    EXPECT_EQ(std::any_cast<std::string>(result_artifact.data), expected_source);

    EXPECT_TRUE(context->source_registry.count(temp_file_path) > 0)
        << "The file path was not registered in the CompilerContext's source registry.";

    EXPECT_EQ(context->source_registry[temp_file_path], expected_source)
        << "The source code saved in the registry does not match the actual file contents.";
}