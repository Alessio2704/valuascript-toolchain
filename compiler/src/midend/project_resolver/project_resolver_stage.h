#pragma once
#include <unordered_set>
#include "resolved_project_artifact.h"
#include "core/compiler_stage.h"
#include "frontend/frontend_orchestrator.h"

namespace valuascript::compiler {
    class ProjectResolverStage : public CompilerStage {
    private:
        std::unordered_set<std::string> resolving_;
        std::unordered_set<std::string> resolved_;
        FrontendOrchestrator frontend_;

        static std::string normalize_path(const std::string &base_file, const std::string &import_path);

        void resolve_recursive(CompilerContext &context, const std::string &current_file,
                               ResolvedProjectArtifact &project);

    public:
        ProjectResolverStage();

        CompilerStageArtifact
        run(CompilerContext &context, const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
