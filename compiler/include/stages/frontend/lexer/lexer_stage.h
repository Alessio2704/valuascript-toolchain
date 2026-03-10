#pragma once
#include "compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    class LexerStage : public CompilerStage {
    public:
        LexerStage();

        CompilerStageArtifact run(std::shared_ptr<CompilerContext> context, const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
