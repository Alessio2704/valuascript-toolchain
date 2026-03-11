#pragma once
#include <map>
#include "compiler_settings.h"
#include "diagnostics_reporter.h"
#include "errors/valuascript_exception.h"

namespace valuascript::compiler {
    using SourceRegistry = std::map<std::string, std::string>;

    class CompilerContext {
    public:
        CompilerSettings settings;
        DiagnosticReporter diagnostics;
        SourceRegistry source_registry;

        void handle_error(const ValuaScriptException &ex);
        void update_source_registry(const std::string &file_path, const std::string &source);
    };
}
