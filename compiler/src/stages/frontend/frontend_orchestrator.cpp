#include "stages/frontend/frontend_orchestrator.h"
#include "compiler_stage/compiler_stage_orchestrator.h"
#include "compiler_stage/compiler_stage.h"
#include "stages/frontend/file_reader/file_reader_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/parser_stage.h"

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
