#include <gtest/gtest.h>
#include "orchestrator/orchestrator.h"
#include "compiler_stage/compiler_stage.h"
#include <stdexcept>
#include <memory>

using namespace valuascript::compiler;

class ParserMock : public CompilerStage {
public:
    ParserMock() : CompilerStage(
        "Parser",
        CompilerStageArtifactCode::Ast,
        {CompilerStageArtifactCode::FilePath}) {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::Ast, {}};
    }
};

class SemanticAnalyserMock : public CompilerStage {
public:
    SemanticAnalyserMock() : CompilerStage(
        "SemanticAnalyser",
        CompilerStageArtifactCode::SymbolTable,
        {CompilerStageArtifactCode::Ast}) {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::SymbolTable, {}};
    }
};

class SymbolTableEnricherMock : public CompilerStage {
public:
    SymbolTableEnricherMock() : CompilerStage(
        "SymbolTableEnricher",
        CompilerStageArtifactCode::EnrichedSymbolTable,
        {CompilerStageArtifactCode::SymbolTable}) {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::EnrichedSymbolTable, {}};
    }
};

class MultipleDependencyStageMock : public CompilerStage {
public:
    MultipleDependencyStageMock() : CompilerStage(
        "MultipleDependencyStage",
        CompilerStageArtifactCode::Bytecode,
        {CompilerStageArtifactCode::Ast, CompilerStageArtifactCode::ValidatedSymbolTable}) {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::Bytecode, {}};
    }
};

class ValidatorMock : public CompilerStage {
public:
    ValidatorMock() : CompilerStage(
        "Validator",
        CompilerStageArtifactCode::ValidatedSymbolTable,
        {CompilerStageArtifactCode::EnrichedSymbolTable}) {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::ValidatedSymbolTable, {}};
    }
};

class LinterMock : public CompilerStage {
public:
    LinterMock() : CompilerStage(
        "Linter",
        CompilerStageArtifactCode::LinterReport,
        {CompilerStageArtifactCode::Ast}) {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::LinterReport, {}};
    }
};

TEST(ValidateOrchestratorPipelineTest, EmptyPipelineDoesNotThrow) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();
    EXPECT_NO_THROW(validate_stages_pipeline(stages));
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineDoesNotThrow) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<ParserMock>());
    stages.push_back(std::make_unique<SemanticAnalyserMock>());
    stages.push_back(std::make_unique<SymbolTableEnricherMock>());

    EXPECT_NO_THROW(validate_stages_pipeline(stages));
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineButWrongOrderThrows) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<ParserMock>());
    stages.push_back(std::make_unique<SymbolTableEnricherMock>());
    stages.push_back(std::make_unique<SemanticAnalyserMock>());

    EXPECT_THROW(validate_stages_pipeline(stages), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineWithMultipleDependencies) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<ParserMock>());
    stages.push_back(std::make_unique<SemanticAnalyserMock>());
    stages.push_back(std::make_unique<SymbolTableEnricherMock>());
    stages.push_back(std::make_unique<ValidatorMock>());
    stages.push_back(std::make_unique<MultipleDependencyStageMock>());

    EXPECT_NO_THROW(validate_stages_pipeline(stages));
}

TEST(ValidateOrchestratorPipelineTest, ValidPipelineWithMultipleDependenciesButMissingOne) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<ParserMock>());
    stages.push_back(std::make_unique<SemanticAnalyserMock>());
    stages.push_back(std::make_unique<MultipleDependencyStageMock>());

    EXPECT_THROW(validate_stages_pipeline(stages), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, MissingDependencyThrowsLogicError1) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<SemanticAnalyserMock>());

    EXPECT_THROW(validate_stages_pipeline(stages), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, MissingDependencyThrowsLogicError2) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<SymbolTableEnricherMock>());

    EXPECT_THROW(validate_stages_pipeline(stages), std::logic_error);
}

TEST(ValidateOrchestratorPipelineTest, MultipleConsumersOfSameArtifactPasses) {
    std::vector<std::unique_ptr<CompilerStage>> stages;
    stages.clear();

    stages.push_back(std::make_unique<ParserMock>());
    stages.push_back(std::make_unique<LinterMock>());
    stages.push_back(std::make_unique<SemanticAnalyserMock>());

    EXPECT_NO_THROW(validate_stages_pipeline(stages));
}
