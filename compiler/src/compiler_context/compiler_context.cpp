#include "compiler_context/compiler_context.h"

namespace valuascript::compiler {

    void CompilerContext::handle_error(const ValuaScriptException &ex) {
        diagnostics.report_error(ex);

        if (settings.fail_fast) {
            throw ex;
        }
    }

    void CompilerContext::update_source_registry(const std::string &file_path, const std::string &source) {
        source_registry[file_path] = source;
    }
}