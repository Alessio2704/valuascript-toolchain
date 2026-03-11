#pragma once

#include "compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    class ParserStage : public CompilerStage {
    public:
        ParserStage();

        CompilerStageArtifact run(const std::shared_ptr<CompilerContext> &context, const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
