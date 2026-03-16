#include "main_orchestrator/main_orchestrator.h"
#include "stages/project_resolver/project_resolver_stage.h"

namespace valuascript::compiler {
    MainOrchestrator::MainOrchestrator() : CompilerStageOrchestrator(
        "MainOrchestrator",
        CompilerStageArtifactCode::ResolvedProject,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<ProjectResolverStage>());
        validate();
    }
}
