#include "compiler_stage/compiler_stage_artifact.h"

namespace valuascript::compiler {
    CompilerStageArtifact extract_artifact(const std::vector<CompilerStageArtifact> &artifacts,
                                           CompilerStageArtifactCode target_code) {
        auto it = std::find_if(artifacts.rbegin(), artifacts.rend(),
                               [target_code](const CompilerStageArtifact &a) { return a.code == target_code; });

        if (it == artifacts.rend()) {
            throw InternalCompilerException(
                InternalErrorCode::MissingArtifactDuringExtraction,
                static_cast<int>(target_code)
            );
        }

        return *it;
    }
}
