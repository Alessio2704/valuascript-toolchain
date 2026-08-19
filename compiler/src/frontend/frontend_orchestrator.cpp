#include "frontend_orchestrator.h"
#include "core/compiler_stage_orchestrator.h"
#include "file_reader/file_reader_stage.h"
#include "lexer/lexer_stage.h"
#include "parser/parser_stage.h"

namespace valuascript::compiler {
    FrontendOrchestrator::FrontendOrchestrator(): CompilerStageOrchestrator(
           "FrontendOrchestrator",
           CompilerStageArtifactCode::Ast,
           {CompilerStageArtifactCode::FilePath}
       ) {
        add_stage(std::make_unique<FileReaderStage>());
        add_stage(std::make_unique<LexerStage>());
        add_stage(std::make_unique<ParserStage>());

        validate();
    }
}
