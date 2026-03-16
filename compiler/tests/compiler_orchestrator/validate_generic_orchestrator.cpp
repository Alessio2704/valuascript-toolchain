#include <gtest/gtest.h>
#include "compiler_stage/compiler_orchestrator.h"
#include "mock_stages.h"

using namespace valuascript::compiler;

class MockOrchestratorSuccessful : public CompilerOrchestrator {
public:
    MockOrchestratorSuccessful() : CompilerOrchestrator(
        "MockOrchestratorSuccessful",
        CompilerStageArtifactCode::ResolvedProject,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<FileReaderMock>());
        add_stage(std::make_unique<LexerMock>());
        add_stage(std::make_unique<ParserMock>());
        add_stage(std::make_unique<ImportResolverMock>());
        validate();
    }
};

class MockOrchestratorMissingDep : public CompilerOrchestrator {
public:
    MockOrchestratorMissingDep() : CompilerOrchestrator(
        "MockOrchestratorMissingDep",
        CompilerStageArtifactCode::Ast,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<FileReaderMock>());
        add_stage(std::make_unique<ParserMock>());
        validate();
    }
};

class MockOrchestratorDuplicateStage : public CompilerOrchestrator {
public:
    MockOrchestratorDuplicateStage() : CompilerOrchestrator(
        "MockOrchestratorDuplicateStage",
        CompilerStageArtifactCode::Ast,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<FileReaderMock>());
        add_stage(std::make_unique<ParserMock>());
        add_stage(std::make_unique<ParserMock>());
        validate();
    }
};

class MockOrchestratorMissingOut : public CompilerOrchestrator {
public:
    MockOrchestratorMissingOut() : CompilerOrchestrator(
        "MockOrchestratorMissingOut",
        CompilerStageArtifactCode::ResolvedProject,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<FileReaderMock>());
        add_stage(std::make_unique<LexerMock>());
        add_stage(std::make_unique<ParserMock>());
        validate();
    }
};

TEST(ValidateOrchestratorTest, MockOrchestratorSuccessfulIsConfiguredCorrectly) {
    MockOrchestratorSuccessful orchestrator;
    SUCCEED();
}

TEST(ValidateOrchestratorTest, ThrowsWhenStageDependencyIsMissing) {
    try {
        MockOrchestratorMissingDep orchestrator;
        FAIL() << "Expected InternalCompilerException to be thrown";
    } catch (const InternalCompilerException &err) {
        EXPECT_EQ(err.get_code(), InternalErrorCode::MissingDependencyInCompilerOrchestrator)
            << "Expected ICE code MissingDependency, but got a different ICE code.";
    } catch (...) {
        FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
    }
}

TEST(ValidateOrchestratorTest, ThrowsWhenPromisedOutputIsMissing) {
    try {
        MockOrchestratorMissingOut orchestrator;
        FAIL() << "Expected InternalCompilerException to be thrown";
    } catch (const InternalCompilerException &err) {
        EXPECT_EQ(err.get_code(), InternalErrorCode::MissingOutputArtifactInCompilerOrchestrator)
            << "Expected ICE code MissingOutputArtifact, but got a different ICE code.";
    } catch (...) {
        FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
    }
}

TEST(ValidateOrchestratorTest, ThrowsWhenAddingDuplicateStage) {
    try {
        MockOrchestratorDuplicateStage orchestrator;
        FAIL() << "Expected InternalCompilerException to be thrown";
    } catch (const InternalCompilerException &err) {
        EXPECT_EQ(err.get_code(), InternalErrorCode::DuplicateStageInOrchestrator)
            << "Expected ICE code DuplicateStageInOrchestrator, but got a different ICE code.";
    } catch (...) {
        FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
    }
}
