#pragma once
#include "compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    class LexerStage : public CompilerStage {
    public:
        LexerStage();

        CompilerStageArtifact run(CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
