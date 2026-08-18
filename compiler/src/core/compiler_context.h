#pragma once
#include "compiler_settings.h"
#include "diagnostics_reporter.h"
#include "valuascript_exception.h"
#include "source_manager.h"

namespace valuascript::compiler {

    class CompilerContext {
    public:
        CompilerSettings settings;
        DiagnosticReporter diagnostics;
        SourceManager source_manager;

        void handle_error(const ValuaScriptException &ex);
    };
}
