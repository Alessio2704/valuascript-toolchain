#pragma once

#include <vector>
#include <any>
#include <set>
#include <string>
#include <memory>
#include <algorithm>

#include "compiler_stage_artifact.h"
#include "compiler_context.h"

namespace valuascript::compiler {
    class CompilerStage {
    private:
        std::string name_;
        CompilerStageArtifactCode output_artifact_;
        std::set<CompilerStageArtifactCode> dependencies_;

    public:
        CompilerStage(std::string name,
                      CompilerStageArtifactCode output,
                      std::set<CompilerStageArtifactCode> required_dependencies)
            : name_(std::move(name)), output_artifact_(output), dependencies_(std::move(required_dependencies)) {
        }

        virtual ~CompilerStage() = default;

        virtual CompilerStageArtifact run(CompilerContext &context,
                                          const std::vector<CompilerStageArtifact> &artifacts) = 0;

        [[nodiscard]] const std::string &get_name() const { return name_; }
        [[nodiscard]] CompilerStageArtifactCode get_output_artifact() const { return output_artifact_; }
        [[nodiscard]] const std::set<CompilerStageArtifactCode> &get_dependencies() const { return dependencies_; }
    };

    std::set<CompilerStageArtifactCode> validate_stages_pipeline(
        const std::vector<std::unique_ptr<CompilerStage> > &stages,
        const std::set<CompilerStageArtifactCode> &initial_inputs);
}
