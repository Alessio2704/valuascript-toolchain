#pragma once
#include <any>
#include <vector>

namespace valuascript::compiler {
    enum class CompilerStageArtifactCode {
        FilePath,
        Ast,
        SymbolTable,
        EnrichedSymbolTable,
        ValidatedSymbolTable,
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
        auto it = std::ranges::find_if(artifacts, [target_code](const CompilerStageArtifact &artifact) {
            return artifact.code == target_code;
        });

        if (it == artifacts.end()) {
            throw std::runtime_error("Extraction Failed: Artifact code not found in pipeline history.");
        }

        try {
            return std::any_cast<ExpectedType>(it->data);
        } catch (const std::bad_any_cast &) {
            throw std::runtime_error("Extraction Failed: Artifact exists, but type mismatch occurred during cast.");
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
                      std::vector<CompilerStageArtifactCode> deps) : name_(std::move(name)),
                                                                     output_artifact_(output),
                                                                     dependencies_(std::move(deps)) {
        }

        virtual ~CompilerStage() = default;

        virtual CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &artifacts) = 0;

        [[nodiscard]] const std::string &get_name() const { return name_; }
        [[nodiscard]] CompilerStageArtifactCode get_output_artifact() const { return output_artifact_; }
        [[nodiscard]] const std::vector<CompilerStageArtifactCode> &get_dependencies() const { return dependencies_; }
    };
}
