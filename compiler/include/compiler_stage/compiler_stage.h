#pragma once

#include <vector>
#include <any>
#include <set>
#include <string>
#include <memory>
#include <algorithm>

#include "compiler_stage_artifact.h"
#include "compiler_context/compiler_context.h"
#include "errors/internal_compiler_exception.h"

namespace valuascript::compiler {
    class CompilerStage {
    private:
        std::string name_;
        CompilerStageArtifactCode output_artifact_;
        std::set<CompilerStageArtifactCode> dependencies_;

    public:
        CompilerStage(std::string name,
                      CompilerStageArtifactCode output,
                      std::set<CompilerStageArtifactCode> deps)
            : name_(std::move(name)), output_artifact_(output), dependencies_(std::move(deps)) {
        }

        virtual ~CompilerStage() = default;

        virtual CompilerStageArtifact run(CompilerContext &context,
                                          const std::vector<CompilerStageArtifact> &artifacts) = 0;

        [[nodiscard]] const std::string &get_name() const { return name_; }
        [[nodiscard]] CompilerStageArtifactCode get_output_artifact() const { return output_artifact_; }
        [[nodiscard]] const std::set<CompilerStageArtifactCode> &get_dependencies() const { return dependencies_; }
    };

    inline std::set<CompilerStageArtifactCode> validate_stages_pipeline(
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
            available_artifacts.insert(stage->get_output_artifact());
        }

        return available_artifacts;
    }
}
