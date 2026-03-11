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
                             std::vector<CompilerStageArtifactCode> deps)
            : CompilerStage(std::move(name), output, std::move(deps)) {
        }

        void add_stage(std::unique_ptr<CompilerStage> stage) {
            stages_.push_back(std::move(stage));
        }

        void validate() const {
            validate_stages_pipeline(stages_, get_dependencies());
        }

        CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &input_artifacts) override;

        CompilerStageArtifact run_from_file(const std::shared_ptr<CompilerContext> &context, const std::string &file_path);
    };
}
