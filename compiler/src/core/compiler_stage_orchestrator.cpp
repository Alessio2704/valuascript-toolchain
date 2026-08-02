#include "compiler_stage_orchestrator.h"
#include <algorithm>

#include "internal_compiler_exception.h"

namespace valuascript::compiler {
    CompilerStageArtifact CompilerStageOrchestrator::run(CompilerContext &context,
                                                         const std::vector<CompilerStageArtifact> &input_artifacts) {
        std::vector<CompilerStageArtifact> local_artifacts = input_artifacts;

        for (const auto &stage: stages_) {
            CompilerStageArtifact new_artifact = stage->run(context, local_artifacts);
            local_artifacts.push_back(std::move(new_artifact));
        }

        auto target_code = get_output_artifact();

        try {
            return extract_artifact(local_artifacts, target_code);
        } catch (const InternalCompilerException &e) {
            if (e.get_code() == InternalErrorCode::MissingArtifactDuringExtraction) {
                throw InternalCompilerException(
                    InternalErrorCode::MissingOutputArtifactInCompilerOrchestrator,
                    get_name(),
                    static_cast<int>(target_code)
                );
            }
            throw;
        }
    }

    CompilerStageArtifact CompilerStageOrchestrator::run_from_file(CompilerContext &context,
                                                                   const std::string &file_path) {
        std::vector<CompilerStageArtifact> initial_artifacts = {
            {.code = CompilerStageArtifactCode::FilePath, .data = file_path}
        };
        return run(context, initial_artifacts);
    }

    void CompilerStageOrchestrator::add_stage(std::unique_ptr<CompilerStage> stage) {
        for (const auto &existing_stage: stages_) {
            if (existing_stage->get_name() == stage->get_name()) {
                throw InternalCompilerException(
                    InternalErrorCode::DuplicateStageInCompilerOrchestrator,
                    get_name(),
                    stage->get_name()
                );
            }
        }
        stages_.push_back(std::move(stage));
    }

    void CompilerStageOrchestrator::validate() const {
        std::set<CompilerStageArtifactCode> produced_artifacts = validate_stages_pipeline(
            stages_, get_dependencies());

        if (!produced_artifacts.contains(get_output_artifact())) {
            throw InternalCompilerException(
                InternalErrorCode::MissingOutputArtifactInCompilerOrchestrator,
                get_name(),
                static_cast<int>(get_output_artifact())
            );
        }
    }
}
