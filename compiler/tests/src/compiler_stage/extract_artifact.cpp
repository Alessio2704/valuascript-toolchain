#include <gtest/gtest.h>
#include "../../../src/core/compiler_stage.h"
#include <stdexcept>
#include <memory>

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    TEST(ArtifactUtilityTest, SuccessfulExtraction) {
        std::vector<CompilerStageArtifact> history;

        history.push_back({CompilerStageArtifactCode::FilePath, std::string("test_file.vs")});

        auto result = extract_artifact(history, CompilerStageArtifactCode::FilePath);
        EXPECT_EQ(result.code, CompilerStageArtifactCode::FilePath);
    }

    TEST(ArtifactUtilityTest, ThrowsOnMissingCode) {
        try {
            std::vector<CompilerStageArtifact> history;
            extract_artifact(history, CompilerStageArtifactCode::FilePath);
        } catch (const InternalCompilerException &err) {
            EXPECT_EQ(err.get_code(), InternalErrorCode::MissingArtifactDuringExtraction)
                << "Expected ICE code MissingArtifactDuringExtraction, but got a different ICE code.";
        } catch (...) {
            FAIL() << "Expected InternalCompilerException, but caught an unknown non-standard exception.";
        }
    }
}
