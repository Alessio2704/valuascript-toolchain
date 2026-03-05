#pragma once
#include <unordered_set>
#include "resolved_project_artifact.h"
#include "compiler_stage/compiler_stage.h"
#include "stages/frontend/frontend_orchestrator.h"

namespace valuascript::compiler {
    class ImportResolverStage : public CompilerStage {
    private:
        std::unordered_set<std::string> resolving_;
        std::unordered_set<std::string> resolved_;
        FrontendOrchestrator frontend_;

        static std::string normalize_path(const std::string& base_file, const std::string& import_path);

        void resolve_recursive(const std::string& current_file, ResolvedProjectArtifact& project);
    public:
        ImportResolverStage();

        CompilerStageArtifact run(const std::vector<CompilerStageArtifact>& artifacts) override;
    };
}
