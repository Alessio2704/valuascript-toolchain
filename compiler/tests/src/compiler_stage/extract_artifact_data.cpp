#include <gtest/gtest.h>
#include "core/compiler_stage.h"
#include <stdexcept>
#include <memory>

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    TEST(ArtifactDataUtilityTest, SuccessfulExtraction) {
        std::vector<CompilerStageArtifact> history;

        history.push_back({.code = CompilerStageArtifactCode::FilePath, .data = std::string("test_file.vs")});

        auto result = extract_artifact_data<std::string>(history, CompilerStageArtifactCode::FilePath);
        EXPECT_EQ(result, "test_file.vs");
    }

    TEST(ArtifactDataUtilityTest, ThrowsOnMissingCode) {
        std::vector<CompilerStageArtifact> history;

        try {
            extract_artifact_data<std::string>(history, CompilerStageArtifactCode::FilePath);
        } catch (const InternalCompilerException &err) {
            EXPECT_EQ(err.get_code(), InternalErrorCode::MissingArtifactDuringExtraction)
                << "Expected ICE code MissingArtifactDuringExtraction, but got a different ICE code.";
        } catch (...) {
            FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
        }
    }

    TEST(ArtifactDataUtilityTest, ThrowsOnTypeMismatch) {
        std::vector<CompilerStageArtifact> history;
        history.push_back({.code = CompilerStageArtifactCode::FilePath, .data = std::string("test_file.vs")});

        try {
            extract_artifact_data<int>(history, CompilerStageArtifactCode::FilePath);
        } catch (const InternalCompilerException &err) {
            EXPECT_EQ(err.get_code(), InternalErrorCode::InvalidArtifactCast)
                << "Expected ICE code InvalidArtifactCast, but got a different ICE code.";
        } catch (...) {
            FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
        }
    }
}
