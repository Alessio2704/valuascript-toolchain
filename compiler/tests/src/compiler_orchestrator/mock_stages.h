#pragma once

#include <gtest/gtest.h>
#include "core/compiler_stage.h"
#include <memory>

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class FileReaderMock : public CompilerStage {
    public:
        FileReaderMock() : CompilerStage(
            "FileReader",
            CompilerStageArtifactCode::SourceCode,
            {CompilerStageArtifactCode::FilePath}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::SourceCode, .data = {}};
        }
    };


    class LexerMock : public CompilerStage {
    public:
        LexerMock() : CompilerStage(
            "Lexer",
            CompilerStageArtifactCode::TokenStream,
            {CompilerStageArtifactCode::SourceCode}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::TokenStream, .data = {}};
        }
    };


    class ParserMock : public CompilerStage {
    public:
        ParserMock() : CompilerStage(
            "Parser",
            CompilerStageArtifactCode::Ast,
            {CompilerStageArtifactCode::FilePath, CompilerStageArtifactCode::TokenStream}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::Ast, .data = {}};
        }
    };

    class ParserMockV2 : public CompilerStage {
    public:
        ParserMockV2() : CompilerStage(
            "ParserMockV2",
            CompilerStageArtifactCode::Ast,
            {CompilerStageArtifactCode::FilePath, CompilerStageArtifactCode::TokenStream}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::Ast, .data = {}};
        }
    };

    class ImportResolverMock : public CompilerStage {
    public:
        ImportResolverMock() : CompilerStage(
            "ImportResolverMock",
            CompilerStageArtifactCode::ResolvedProject,
            {CompilerStageArtifactCode::Ast, CompilerStageArtifactCode::FilePath}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::ResolvedProject, .data = {}};
        }
    };

    class SemanticAnalyserMock : public CompilerStage {
    public:
        SemanticAnalyserMock() : CompilerStage(
            "SemanticAnalyser",
            CompilerStageArtifactCode::SymbolTable,
            {CompilerStageArtifactCode::ResolvedProject}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::SymbolTable, .data = {}};
        }
    };

    class BytecodeEmitterMock : public CompilerStage {
    public:
        BytecodeEmitterMock() : CompilerStage(
            "SymbolTableEnricher",
            CompilerStageArtifactCode::Bytecode,
            {CompilerStageArtifactCode::SymbolTable}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::Bytecode, .data = {}};
        }
    };

    class LinterMock : public CompilerStage {
    public:
        LinterMock() : CompilerStage(
            "Linter",
            CompilerStageArtifactCode::LinterReport,
            {CompilerStageArtifactCode::ResolvedProject}) {
        }

        CompilerStageArtifact run([[maybe_unused]] CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &) override {
            return {.code = CompilerStageArtifactCode::LinterReport, .data = {}};
        }
    };
}
