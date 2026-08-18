#include "compiler_context.h"

namespace valuascript::compiler {

    void CompilerContext::handle_error(const ValuaScriptException &ex) {
        diagnostics.report_error(ex);

        if (settings.fail_fast) {
            throw ex;
        }
    }
}