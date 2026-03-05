#include "main_orchestrator/main_orchestrator.h"
#include "stages/import_resolver/import_resolver_stage.h"

namespace valuascript::compiler {
    MainOrchestrator::MainOrchestrator() : CompilerOrchestrator(
        "MainOrchestrator",
        CompilerStageArtifactCode::ResolvedProject,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<ImportResolverStage>());
        validate();
    }
}
