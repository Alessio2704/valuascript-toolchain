#pragma once
#include "compiler_stage/compiler_orchestrator.h"
#include "stages/frontend/frontend_orchestrator.h"

namespace valuascript::compiler {

    class MainOrchestrator : public CompilerOrchestrator {
    public:
        MainOrchestrator();
    };
}