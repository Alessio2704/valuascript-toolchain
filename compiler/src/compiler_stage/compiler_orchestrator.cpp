#include "compiler_stage/compiler_orchestrator.h"
#include <algorithm>

namespace valuascript::compiler {
    CompilerStageArtifact CompilerOrchestrator::run(CompilerContext &context,
                                                    const std::vector<CompilerStageArtifact> &input_artifacts) {
        std::vector<CompilerStageArtifact> local_artifacts = input_artifacts;

        for (const auto &stage: stages_) {
            CompilerStageArtifact new_artifact = stage->run(context, local_artifacts);
            local_artifacts.push_back(std::move(new_artifact));
        }

        auto target_code = get_output_artifact();

        auto it = std::find_if(local_artifacts.rbegin(), local_artifacts.rend(),
                               [target_code](const CompilerStageArtifact &a) { return a.code == target_code; });

        if (it == local_artifacts.rend()) {
            throw InternalCompilerException(
                InternalErrorCode::MissingOutputArtifactInCompilerOrchestrator,
                get_name(),
                static_cast<int>(get_output_artifact())
            );
        }

        return *it;
    }

    CompilerStageArtifact CompilerOrchestrator::run_from_file(CompilerContext &context,
                                                              const std::string &file_path) {
        std::vector<CompilerStageArtifact> initial_artifacts = {
            {CompilerStageArtifactCode::FilePath, file_path}
        };
        return run(context, initial_artifacts);
    }
}
