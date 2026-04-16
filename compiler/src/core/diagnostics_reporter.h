#pragma once
#include <vector>
#include "valuascript_exception.h"

namespace valuascript::compiler {
    class DiagnosticReporter {
    private:
        std::vector<ValuaScriptException> errors_;

    public:
        void report_error(const ValuaScriptException &exception) {
            errors_.push_back(exception);
        }

        [[nodiscard]] bool has_errors() const { return !errors_.empty(); }
        [[nodiscard]] const std::vector<ValuaScriptException> &get_errors() const { return errors_; }
    };
}
