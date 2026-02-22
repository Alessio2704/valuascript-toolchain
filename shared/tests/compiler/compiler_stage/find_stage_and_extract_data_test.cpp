#include <gtest/gtest.h>
#include "compiler/compiler_stage/compiler_stage.h"
#include <string>
#include <vector>

using namespace valuascript;

TEST(ArtifactUtilityTest, SuccessfulExtraction)
{
    std::vector<CompilerStageArtifact> history;

    history.push_back({CompilerStageArtifactCode::FilePath, std::string("test_file.vs")});

    auto result = extract_artifact_data<std::string>(history, CompilerStageArtifactCode::FilePath);
    EXPECT_EQ(result, "test_file.vs");
}

TEST(ArtifactUtilityTest, ThrowsOnMissingCode)
{
    std::vector<CompilerStageArtifact> history;

    EXPECT_THROW(
        extract_artifact_data<std::string>(history, CompilerStageArtifactCode::Ast),
        std::runtime_error);
}

TEST(ArtifactUtilityTest, ThrowsOnTypeMismatch)
{
    std::vector<CompilerStageArtifact> history;
    history.push_back({CompilerStageArtifactCode::FilePath, std::string("test_file.vs")});

    EXPECT_THROW(
        extract_artifact_data<int>(history, CompilerStageArtifactCode::FilePath),
        std::runtime_error);
}