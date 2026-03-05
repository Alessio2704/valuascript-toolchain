#pragma once
#include "compiler_stage/compiler_orchestrator.h"
#include "stages/frontend/file_reader/file_reader_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/parser_stage.h"

namespace valuascript::compiler {
    class FrontendOrchestrator : public CompilerOrchestrator {
    public:
        FrontendOrchestrator();
    };
}
