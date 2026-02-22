#include <gtest/gtest.h>
#include "core/orchestrator/orchestrator.h"
#include "compiler/compiler_stage/compiler_stage.h"
#include <stdexcept>
#include <memory>

using namespace valuascript::compiler;

class Parser : public CompilerStage
{
public:
    Parser() : CompilerStage(
                   "Parser",
                   CompilerStageArtifactCode::Ast,
                   {CompilerStageArtifactCode::FilePath}) {}

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override
    {
        return {CompilerStageArtifactCode::Ast, {}};
    }
};

class SemanticAnalyser : public CompilerStage
{
public:
    SemanticAnalyser() : CompilerStage(
                             "SemanticAnalyser",
                             CompilerStageArtifactCode::SymbolTable,
                             {CompilerStageArtifactCode::Ast}) {}

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override
    {
        return {CompilerStageArtifactCode::SymbolTable, {}};
    }
};

class SymbolTableEnricher : public CompilerStage
{
public:
    SymbolTableEnricher() : CompilerStage(
                                "SymbolTableEnricher",
                                CompilerStageArtifactCode::EnrichedSymbolTable,
                                {CompilerStageArtifactCode::SymbolTable}) {}

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override
    {
        return {CompilerStageArtifactCode::EnrichedSymbolTable, {}};
    }
};

class MultipleDependencyStage : public CompilerStage
{
public:
    MultipleDependencyStage() : CompilerStage(
                                    "MultipleDependencyStage",
                                    CompilerStageArtifactCode::Bytecode,
                                    {CompilerStageArtifactCode::Ast, CompilerStageArtifactCode::ValidatedSymbolTable}) {}

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override
    {
        return {CompilerStageArtifactCode::Bytecode, {}};
    }
};

class Validator : public CompilerStage
{
public:
    Validator() : CompilerStage(
                      "Validator",
                      CompilerStageArtifactCode::ValidatedSymbolTable,
                      {CompilerStageArtifactCode::EnrichedSymbolTable}) {}

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override
    {
        return {CompilerStageArtifactCode::ValidatedSymbolTable, {}};
    }
};

class Linter : public CompilerStage
{
public:
    Linter() : CompilerStage(
                   "Linter",
                   CompilerStageArtifactCode::LinterReport,
                   {CompilerStageArtifactCode::Ast})
    {
    }

    CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &) override
    {
        return {CompilerStageArtifactCode::LinterReport, {}};
    }
};

TEST(OrchestratorTest, EmptyPipelineDoesNotThrow)
{
    Orchestrator orchestrator;

    EXPECT_NO_THROW(orchestrator.validate_stages_pipeline());
}

TEST(OrchestratorTest, ValidPipelineDoesNotThrow)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<Parser>());
    orchestrator.add_stage(std::make_unique<SemanticAnalyser>());
    orchestrator.add_stage(std::make_unique<SymbolTableEnricher>());

    EXPECT_NO_THROW(orchestrator.validate_stages_pipeline());
}

TEST(OrchestratorTest, ValidPipelineButWrongOrderThrows)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<Parser>());
    orchestrator.add_stage(std::make_unique<SymbolTableEnricher>());
    orchestrator.add_stage(std::make_unique<SemanticAnalyser>());

    EXPECT_THROW(orchestrator.validate_stages_pipeline(), std::logic_error);
}

TEST(OrchestratorTest, ValidPipelineWithMultipleDependencies)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<Parser>());
    orchestrator.add_stage(std::make_unique<SemanticAnalyser>());
    orchestrator.add_stage(std::make_unique<SymbolTableEnricher>());
    orchestrator.add_stage(std::make_unique<Validator>());
    orchestrator.add_stage(std::make_unique<MultipleDependencyStage>());

    EXPECT_NO_THROW(orchestrator.validate_stages_pipeline());
}

TEST(OrchestratorTest, ValidPipelineWithMultipleDependenciesButMissingOne)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<Parser>());
    orchestrator.add_stage(std::make_unique<SemanticAnalyser>());
    orchestrator.add_stage(std::make_unique<MultipleDependencyStage>());

    EXPECT_THROW(orchestrator.validate_stages_pipeline(), std::logic_error);
}

TEST(OrchestratorTest, MissingDependencyThrowsLogicError1)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<SemanticAnalyser>());

    EXPECT_THROW(orchestrator.validate_stages_pipeline(), std::logic_error);
}

TEST(OrchestratorTest, MissingDependencyThrowsLogicError2)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<SymbolTableEnricher>());

    EXPECT_THROW(orchestrator.validate_stages_pipeline(), std::logic_error);
}

TEST(OrchestratorTest, MultipleConsumersOfSameArtifactPasses)
{
    Orchestrator orchestrator;

    orchestrator.add_stage(std::make_unique<Parser>());
    orchestrator.add_stage(std::make_unique<Linter>());
    orchestrator.add_stage(std::make_unique<SemanticAnalyser>());

    EXPECT_NO_THROW(orchestrator.validate_stages_pipeline());
}