#include "stages/frontend/frontend_orchestrator.h"
#include "compiler_stage/compiler_orchestrator.h"
#include "compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    FrontendOrchestrator::FrontendOrchestrator(): CompilerOrchestrator(
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
