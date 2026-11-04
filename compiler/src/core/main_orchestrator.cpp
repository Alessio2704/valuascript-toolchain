#include "main_orchestrator.h"
#include "midend/project_resolver/project_resolver_stage.h"
#include "midend/symbol_resolver/symbol_resolver_stage.h"

namespace valuascript::compiler {
    MainOrchestrator::MainOrchestrator() : CompilerStageOrchestrator(
        "MainOrchestrator",
        CompilerStageArtifactCode::SymbolTable,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<ProjectResolverStage>());
        add_stage(std::make_unique<SymbolResolverStage>());
        validate();
    }
}
