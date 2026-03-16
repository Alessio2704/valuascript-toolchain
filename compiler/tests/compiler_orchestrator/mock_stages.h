#pragma once

#include <gtest/gtest.h>
#include "compiler_stage/compiler_stage.h"
#include <memory>

using namespace valuascript::compiler;

class FileReaderMock : public CompilerStage {
public:
    FileReaderMock() : CompilerStage(
        "FileReader",
        CompilerStageArtifactCode::SourceCode,
        {CompilerStageArtifactCode::FilePath}) {
    }

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
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

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::TokenStream, {}};
    }
};


class ParserMock : public CompilerStage {
public:
    ParserMock() : CompilerStage(
        "Parser",
        CompilerStageArtifactCode::Ast,
        {CompilerStageArtifactCode::FilePath, CompilerStageArtifactCode::TokenStream}) {
    }

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
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

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
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

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
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

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
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

    CompilerStageArtifact run([[maybe_unused]] const std::shared_ptr<CompilerContext> &context,
                              const std::vector<CompilerStageArtifact> &) override {
        return {CompilerStageArtifactCode::LinterReport, {}};
    }
};
