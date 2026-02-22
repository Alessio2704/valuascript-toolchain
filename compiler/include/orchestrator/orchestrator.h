#pragma once
#include <vector>
#include <memory>
#include "compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    using namespace valuascript;

    class Orchestrator {
        std::vector<std::unique_ptr<CompilerStage> > stages_pipeline_;
        std::vector<CompilerStageArtifact> pipeline_artifacts_;

        void initialise_file_path(const std::string &file_path);

    public:
        Orchestrator();

        void add_stage(std::unique_ptr<CompilerStage> stage) {
            stages_pipeline_.push_back(std::move(stage));
        }

        void validate_stages_pipeline() const;

        void run(const std::string &file_path);
    };
}
