#pragma once
#include <vector>
#include "errors/valuascript_exception.h"

namespace valuascript::compiler {

    struct CompilerSettings {
        bool fail_fast = true;
    };

    class DiagnosticReporter {
    private:
        std::vector<ValuaScriptException> errors_;

    public:
        void report_error(const ValuaScriptException& exception) {
            errors_.push_back(exception);
        }

        [[nodiscard]] bool has_errors() const { return !errors_.empty(); }
        [[nodiscard]] const std::vector<ValuaScriptException>& get_errors() const { return errors_; }
    };

    struct CompilerContext {
        CompilerSettings settings;
        DiagnosticReporter diagnostics;
    };
}