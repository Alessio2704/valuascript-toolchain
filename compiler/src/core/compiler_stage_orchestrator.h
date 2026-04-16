#pragma once
#include "compiler_stage.h"
#include <vector>
#include <memory>
#include <string>

namespace valuascript::compiler {
    class CompilerStageOrchestrator : public CompilerStage {
    protected:
        std::vector<std::unique_ptr<CompilerStage> > stages_;

    public:
        CompilerStageOrchestrator(std::string name,
                             CompilerStageArtifactCode output,
                             std::set<CompilerStageArtifactCode> deps)
            : CompilerStage(std::move(name), output, std::move(deps)) {
        }

        void add_stage(std::unique_ptr<CompilerStage> stage);

        void validate() const;

        CompilerStageArtifact run(CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &input_artifacts) override;

        CompilerStageArtifact run_from_file(CompilerContext &context,
                                            const std::string &file_path);
    };
}
