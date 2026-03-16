#include "errors/error_formatter.h"
#include "errors/internal_compiler_exception.h"

namespace valuascript::compiler {
    std::string_view get_error_template(InternalErrorCode code) {
        switch (code) {
            case InternalErrorCode::MissingDependencyInCompilerOrchestrator:
                return
                        "Invalid Pipeline Configuration. Stage '{}' requires Artifact Code '{}', but it is not available from previous stages.";
            case InternalErrorCode::MissingOutputArtifactInCompilerOrchestrator:
                return
                        "Orchestrator Validation Failed. The pipeline inside '{}' does not produce the promised output artifact '{}'.";
            case InternalErrorCode::DuplicateStageInCompilerOrchestrator:
                return
                        "Adding Stage to Orchestrator Failed. The orchestrator '{}' attempted to add a stage '{}' which was already present in the pipeline.";
            case InternalErrorCode::DuplicateOutputArtifactInCompilerOrchestrator:
                return
                        "Invalid Pipeline Configuration. Stage '{}' produces Artifact Code '{}', but this artifact has already been produced by a previous stage or provided as an initial input.";
            case InternalErrorCode::MissingArtifactDuringExtraction:
                return "Extraction Failed: Artifact Code '{}' was not found in the pipeline history.";
            case InternalErrorCode::InvalidArtifactCast:
                return
                        "Extraction Failed: Artifact Code '{}' exists, but could not be cast to the requested type '{}'.";
        }

        return "An unknown internal compiler error occurred.";
    }
}
