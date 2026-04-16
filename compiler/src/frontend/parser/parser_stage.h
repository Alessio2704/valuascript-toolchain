#pragma once

#include "core/compiler_stage.h"

namespace valuascript::compiler {
    class ParserStage : public CompilerStage {
    public:
        ParserStage();

        CompilerStageArtifact run(CompilerContext &context, const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
