#pragma once
#include <map>
#include <string>
#include <vector>
#include "valuascript_exception.h"
#include "compiler_context.h"

namespace valuascript::compiler {
    class DiagnosticFormatter {
    public:
        static constexpr const char *RESET = "\033[0m";
        static constexpr const char *RED = "\033[1;31m";
        static constexpr const char *BLUE = "\033[1;34m";
        static constexpr const char *BOLD = "\033[1m";

        static void print_errors(const std::vector<ValuaScriptException> &errors,
                                 const SourceManager &source_manager);

        static std::string format_error(const ValuaScriptException &err, std::string_view source_code);

    private:
        static std::vector<std::string_view> extract_lines(std::string_view source, size_t line_start, size_t line_end);
    };
}
