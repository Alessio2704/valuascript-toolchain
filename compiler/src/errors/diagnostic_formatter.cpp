#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "errors/diagnostic_formatter.h"

namespace valuascript::compiler {
    void DiagnosticFormatter::print_errors(const std::vector<ValuaScriptException> &errors,
                                           const SourceRegistry &registry) {
        for (const auto &err: errors) {
            const auto& loc = err.get_location();
            std::string source_code;

            if (registry.contains(loc.file_path)) {
                source_code = registry.at(loc.file_path);
            }

            std::cout << format_error(err, source_code) << "\n\n";
        }
    }

    std::string DiagnosticFormatter::format_error(const ValuaScriptException& err, const std::string& source_code) {
        std::ostringstream output;
        const auto& span = err.get_location();

        output << RED << BOLD << "error[E" << static_cast<int>(err.get_code()) << "]: "
               << RESET << BOLD << err.what() << RESET << "\n";

        output << BLUE << "  --> " << RESET
               << span.file_path << ":" << span.line_start << ":" << span.column_start << "\n";

        std::string target_line = extract_line(source_code, span.line_start);

        output << BLUE << " " << span.line_start << " | " << RESET << target_line << "\n";

        std::string prefix = " " + std::to_string(span.line_start) + " | ";
        std::string empty_margin(prefix.length(), ' ');

        size_t spaces = (span.column_start > 1) ? span.column_start - 1 : 0;
        std::string indent(spaces, ' ');

        size_t squiggle_length = 1;

        if (span.line_start == span.line_end && span.column_end > span.column_start) {
            squiggle_length = span.column_end - span.column_start;
        }

        std::string squiggles = "^";
        if (squiggle_length > 1) {
            squiggles += std::string(squiggle_length - 1, '~');
        }

        output << empty_margin << indent << RED << squiggles << RESET;

        return output.str();
    }

    std::string DiagnosticFormatter::extract_line(const std::string &source, size_t line_number) {
        std::istringstream stream(source);
        std::string line;
        size_t current = 1;
        while (std::getline(stream, line)) {
            if (current == line_number) return line;
            current++;
        }
        return "<source line not available>";
    }
}
