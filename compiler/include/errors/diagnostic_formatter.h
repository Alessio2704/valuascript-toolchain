#pragma once
#include <map>
#include <string>
#include <vector>
#include "valuascript_exception.h"

namespace valuascript::compiler {
    class DiagnosticFormatter {
    public:
        static constexpr const char *RESET = "\033[0m";
        static constexpr const char *RED = "\033[1;31m";
        static constexpr const char *BLUE = "\033[1;34m";
        static constexpr const char *BOLD = "\033[1m";

        static void print_errors(const std::vector<ValuaScriptException> &errors,
                                               const std::map<std::string, std::string> &registry);

        static std::string format_error(const ValuaScriptException &err, const std::string &source_code);

    private:
        static std::string extract_line(const std::string &source, size_t line_number);
    };
}
