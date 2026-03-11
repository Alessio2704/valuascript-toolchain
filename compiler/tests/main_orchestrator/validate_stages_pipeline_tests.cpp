#include <gtest/gtest.h>
#include "compiler_stage/compiler_stage.h"
#include <stdexcept>
#include <memory>

using namespace valuascript::compiler;

class FileReaderMock : public CompilerStage {
public:
    FileReaderMock() : CompilerStage(
        "FileReader",
        CompilerStageArtifactCode::SourceCode,
        {CompilerStageArtifactCode::FilePath}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::SourceCode, {}};
    }
};


class LexerMock : public CompilerStage {
public:
    LexerMock() : CompilerStage(
        "Lexer",
        CompilerStageArtifactCode::TokenStream,
        {CompilerStageArtifactCode::SourceCode}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::TokenStream, {}};
    }
};


class ParserMock : public CompilerStage {
public:
    ParserMock() : CompilerStage(
        "Parser",
        CompilerStageArtifactCode::Ast,
        {CompilerStageArtifactCode::FilePath, CompilerStageArtifactCode::SourceCode}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::Ast, {}};
    }
};

class ImportResolverMock : public CompilerStage {
public:
    ImportResolverMock() : CompilerStage(
        "ImportResolverMock",
        CompilerStageArtifactCode::ResolvedProject,
        {CompilerStageArtifactCode::Ast, CompilerStageArtifactCode::FilePath}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::ResolvedProject, {}};
    }
};

class SemanticAnalyserMock : public CompilerStage {
public:
    SemanticAnalyserMock() : CompilerStage(
        "SemanticAnalyser",
        CompilerStageArtifactCode::SymbolTable,
        {CompilerStageArtifactCode::ResolvedProject}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::SymbolTable, {}};
    }
};

class BytecodeEmitterMock : public CompilerStage {
public:
    BytecodeEmitterMock() : CompilerStage(
        "SymbolTableEnricher",
        CompilerStageArtifactCode::Bytecode,
        {CompilerStageArtifactCode::SymbolTable}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::Bytecode, {}};
    }
};

class LinterMock : public CompilerStage {
public:
    LinterMock() : CompilerStage(
        "Linter",
        CompilerStageArtifactCode::LinterReport,
        {CompilerStageArtifactCode::ResolvedProject}) {
    }

    CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::LinterReport, {}};
    }
};

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
