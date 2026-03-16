#pragma once
#include "compiler_stage/compiler_stage.h"

namespace valuascript::compiler {
    class FileReaderStage : public CompilerStage {
    public:
        FileReaderStage();

        CompilerStageArtifact run(CompilerContext &context,
                                  const std::vector<CompilerStageArtifact> &artifacts) override;
    };
}
