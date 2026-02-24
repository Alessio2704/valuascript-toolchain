#pragma once

#include "../../compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    class ParserStage : public CompilerStage {
    public:
        ParserStage();

        CompilerStageArtifact run(const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
