#include <gtest/gtest.h>
#include "mock_stages.h"
#include <stdexcept>
#include <memory>

using namespace valuascript::compiler;

TEST(ValidateOrchestratorPipelineTest, EmptyPipelineDoesNotThrow) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();
    EXPECT_NO_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}));
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineDoesNotThrow) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();

    stages.push_back(std::make_unique<FileReaderMock>());
    stages.push_back(std::make_unique<LexerMock>());
    stages.push_back(std::make_unique<ParserMock>());

    EXPECT_NO_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}));
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineButWrongOrderThrows) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();

    stages.push_back(std::make_unique<LexerMock>());
    stages.push_back(std::make_unique<ImportResolverMock>());
    stages.push_back(std::make_unique<ParserMock>());

    EXPECT_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineWithMultipleDependenciesButMissingOne) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();

    stages.push_back(std::make_unique<ImportResolverMock>());
    stages.push_back(std::make_unique<ParserMock>());

    EXPECT_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, MissingDependencyThrowsLogicError1) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();

    stages.push_back(std::make_unique<SemanticAnalyserMock>());

    EXPECT_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, MissingDependencyThrowsLogicError2) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();

    stages.push_back(std::make_unique<BytecodeEmitterMock>());

    EXPECT_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, MultipleConsumersOfSameArtifactPasses) {
    std::vector<std::unique_ptr<CompilerStage> > stages;
    stages.clear();

    stages.push_back(std::make_unique<FileReaderMock>());
    stages.push_back(std::make_unique<LexerMock>());
    stages.push_back(std::make_unique<ParserMock>());
    stages.push_back(std::make_unique<ImportResolverMock>());
    stages.push_back(std::make_unique<SemanticAnalyserMock>());
    stages.push_back(std::make_unique<LinterMock>());

    EXPECT_NO_THROW(validate_stages_pipeline(stages, {CompilerStageArtifactCode::FilePath}));
}
