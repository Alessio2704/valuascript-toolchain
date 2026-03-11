#pragma once

#include <vector>
#include <any>
#include <set>
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "compiler_context.h"

namespace valuascript::compiler {
    enum class CompilerStageArtifactCode {
        FilePath,
        SourceCode,
        TokenStream,
        Ast,
        ResolvedProject,
        SymbolTable,
        Bytecode,
        LinterReport
    };

    struct CompilerStageArtifact {
        CompilerStageArtifactCode code;
        std::any data;
    };

    template<typename ExpectedType>
    ExpectedType extract_artifact_data(const std::vector<CompilerStageArtifact> &artifacts,
                                       CompilerStageArtifactCode target_code) {
        auto it = std::find_if(artifacts.begin(), artifacts.end(), [target_code](const CompilerStageArtifact &a) {
            return a.code == target_code;
        });

        if (it == artifacts.end()) {
            throw std::runtime_error("Extraction Failed: Artifact code not found in pipeline history.");
        }

        try {
            return std::any_cast<ExpectedType>(it->data);
        } catch (const std::bad_any_cast &) {
            throw std::runtime_error("Extraction Failed: Artifact type mismatch during cast.");
        }
    }

    class CompilerStage {
    private:
        std::string name_;
        CompilerStageArtifactCode output_artifact_;
        std::vector<CompilerStageArtifactCode> dependencies_;

    public:
        CompilerStage(std::string name,
                      CompilerStageArtifactCode output,
                      std::vector<CompilerStageArtifactCode> deps)
            : name_(std::move(name)), output_artifact_(output), dependencies_(std::move(deps)) {
        }

        virtual ~CompilerStage() = default;

        virtual CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &artifacts) = 0;

        [[nodiscard]] const std::string &get_name() const { return name_; }
        [[nodiscard]] CompilerStageArtifactCode get_output_artifact() const { return output_artifact_; }
        [[nodiscard]] const std::vector<CompilerStageArtifactCode> &get_dependencies() const { return dependencies_; }
    };

    inline void validate_stages_pipeline(const std::vector<std::unique_ptr<CompilerStage> > &stages,
                                         const std::vector<CompilerStageArtifactCode> &initial_inputs) {
        std::set<CompilerStageArtifactCode> available_artifacts(initial_inputs.begin(), initial_inputs.end());

        for (const auto &stage: stages) {
            for (const auto &dependency: stage->get_dependencies()) {
                if (!available_artifacts.contains(dependency)) {
                    throw std::logic_error(
                        "Invalid Pipeline:\nStage '" + stage->get_name() +
                        "' requires Artifact Code '" + std::to_string(static_cast<int>(dependency)) +
                        "', but it is not available."
                    );
                }
            }
            available_artifacts.insert(stage->get_output_artifact());
        }
    }
}
