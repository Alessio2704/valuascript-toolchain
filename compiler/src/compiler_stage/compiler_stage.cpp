#include "compiler_stage/compiler_stage.h"
#include "errors/internal_compiler_exception.h"

namespace valuascript::compiler {

    std::set<CompilerStageArtifactCode> validate_stages_pipeline(
        const std::vector<std::unique_ptr<CompilerStage> > &stages,
        const std::set<CompilerStageArtifactCode> &initial_inputs) {
        std::set<CompilerStageArtifactCode> available_artifacts(initial_inputs.begin(), initial_inputs.end());
        for (const auto &stage: stages) {
            for (const auto &dependency: stage->get_dependencies()) {
                if (!available_artifacts.contains(dependency)) {
                    throw InternalCompilerException(
                        InternalErrorCode::MissingDependencyInCompilerOrchestrator,
                        stage->get_name(),
                        static_cast<int>(dependency)
                    );
                }
            }

            CompilerStageArtifactCode output = stage->get_output_artifact();
            if (available_artifacts.contains(output)) {
                throw InternalCompilerException(
                    InternalErrorCode::DuplicateOutputArtifactInCompilerOrchestrator,
                    stage->get_name(),
                    static_cast<int>(output)
                );
            }

            available_artifacts.insert(output);
        }

        return available_artifacts;
    }
}
