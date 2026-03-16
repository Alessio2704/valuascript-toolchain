#include <gtest/gtest.h>
#include "compiler_stage/compiler_stage_orchestrator.h"
#include "mock_stages.h"
#include "errors/internal_compiler_exception.h"

using namespace valuascript::compiler;

class MockOrchestratorSuccessful : public CompilerStageOrchestrator {
public:
    MockOrchestratorSuccessful() : CompilerStageOrchestrator(
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

class MockOrchestratorMissingDep : public CompilerStageOrchestrator {
public:
    MockOrchestratorMissingDep() : CompilerStageOrchestrator(
        "MockOrchestratorMissingDep",
        CompilerStageArtifactCode::Ast,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<FileReaderMock>());
        add_stage(std::make_unique<ParserMock>());
        validate();
    }
};

class MockOrchestratorDuplicateStage : public CompilerStageOrchestrator {
public:
    MockOrchestratorDuplicateStage() : CompilerStageOrchestrator(
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

class MockOrchestratorMissingOut : public CompilerStageOrchestrator {
public:
    MockOrchestratorMissingOut() : CompilerStageOrchestrator(
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

class MockOrchestratorSameOutTwice : public CompilerStageOrchestrator {
public:
    MockOrchestratorSameOutTwice() : CompilerStageOrchestrator(
        "MockOrchestratorSameOutTwice",
        CompilerStageArtifactCode::ResolvedProject,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<FileReaderMock>());
        add_stage(std::make_unique<LexerMock>());
        add_stage(std::make_unique<ParserMock>());
        add_stage(std::make_unique<ParserMockV2>());
        add_stage(std::make_unique<ImportResolverMock>());
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
        EXPECT_EQ(err.get_code(), InternalErrorCode::DuplicateStageInCompilerOrchestrator)
            << "Expected ICE code DuplicateStageInCompilerOrchestrator, but got a different ICE code.";
    } catch (...) {
        FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
    }
}

TEST(ValidateOrchestratorTest, ThrowsWhenMultipleStagesOutputSameArtifact) {
    try {
        MockOrchestratorSameOutTwice orchestrator;
        FAIL() << "Expected InternalCompilerException to be thrown";
    } catch (const InternalCompilerException &err) {
        EXPECT_EQ(err.get_code(), InternalErrorCode::DuplicateOutputArtifactInCompilerOrchestrator)
            << "Expected ICE code DuplicateOutputArtifactInCompilerOrchestrator, but got a different ICE code.";
    } catch (...) {
        FAIL() << "Expected InternalCompilerException, but a different exception was thrown";
    }
}
