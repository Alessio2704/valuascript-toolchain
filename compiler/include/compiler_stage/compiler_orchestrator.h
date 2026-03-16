#pragma once
#include "compiler_stage.h"
#include <vector>
#include <memory>
#include <string>

namespace valuascript::compiler {
    class CompilerOrchestrator : public CompilerStage {
    protected:
        std::vector<std::unique_ptr<CompilerStage> > stages_;

    public:
        CompilerOrchestrator(std::string name,
                             CompilerStageArtifactCode output,
                             std::set<CompilerStageArtifactCode> deps)
            : CompilerStage(std::move(name), output, std::move(deps)) {
        }

        void add_stage(std::unique_ptr<CompilerStage> stage) {
            for (const auto& existing_stage : stages_) {
                if (existing_stage->get_name() == stage->get_name()) {
                    throw InternalCompilerException(
                        InternalErrorCode::DuplicateStageInOrchestrator,
                        get_name(),
                        stage->get_name()
                    );
                }
            }
            stages_.push_back(std::move(stage));
        }

        void validate() const {
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

        CompilerStageArtifact run(CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &input_artifacts) override;

        CompilerStageArtifact run_from_file(CompilerContext &context,
                                            const std::string &file_path);
    };
}
