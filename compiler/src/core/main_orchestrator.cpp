#include "main_orchestrator.h"
#include "midend/project_resolver/project_resolver_stage.h"
#include "midend/symbol_resolver/symbol_resolver_stage.h"
#include "midend/type_checker/type_checker_stage.h"

namespace valuascript::compiler {
    MainOrchestrator::MainOrchestrator() : CompilerStageOrchestrator(
        "MainOrchestrator",
        CompilerStageArtifactCode::TypeTable,
        {CompilerStageArtifactCode::FilePath}
    ) {
        add_stage(std::make_unique<ProjectResolverStage>());
        add_stage(std::make_unique<SymbolResolverStage>());
        add_stage(std::make_unique<TypeCheckerStage>());
        validate();
    }
}
