#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "errors/diagnostic_formatter.h"

namespace valuascript::compiler {
    void DiagnosticFormatter::print_errors(const std::vector<ValuaScriptException> &errors,
                                           const SourceRegistry &source_registry) {
        for (const auto &err: errors) {
            const auto &loc = err.get_span();
            std::string source_code;

            if (source_registry.contains(loc.file_path)) {
                source_code = source_registry.at(loc.file_path);
            }

            std::cout << format_error(err, source_code) << "\n\n";
        }
    }

    std::string DiagnosticFormatter::format_error(const ValuaScriptException &err, const std::string &source_code) {
        std::ostringstream output;
        const auto &span = err.get_span();

        output << RED << BOLD << "error[E" << static_cast<int>(err.get_code()) << "]: "
                << RESET << BOLD << err.what() << RESET << "\n";

        output << BLUE << "  --> " << RESET
                << span.file_path << ":" << span.line_start << ":" << span.column_start << "\n";

        size_t start = span.line_start;
        size_t end = (span.line_end > 0) ? span.line_end : start;
        if (end < start) end = start;

        std::vector<std::string> lines = extract_lines(source_code, start, end);

        auto padding = std::to_string(end).length();

        auto print_margin = [&](size_t line_num) {
            std::string num_str = std::to_string(line_num);
            std::string pad(padding - num_str.length(), ' ');
            return " " + pad + num_str + " | ";
        };

        std::string empty_margin(padding + 4, ' ');
        std::string pipe_margin(padding + 2, ' ');

        if (start == end) {
            std::string target_line = lines.empty() ? "<source line not available>" : lines[0];
            output << BLUE << print_margin(start) << RESET << target_line << "\n";

            size_t spaces = (span.column_start > 1) ? span.column_start - 1 : 0;
            std::string indent(spaces, ' ');

            size_t squiggle_length = 1;
            if (span.column_end > span.column_start) {
                squiggle_length = span.column_end - span.column_start;
            }

            std::string squiggles = "^";
            if (squiggle_length > 1) {
                squiggles += std::string(squiggle_length - 1, '~');
            }

            output << empty_margin << indent << RED << squiggles << RESET;
        } else {
            if (lines.empty()) {
                output << BLUE << print_margin(start) << RESET << "<source line not available>\n";
            } else {
                constexpr size_t MAX_LINES = 6;
                bool truncated = lines.size() > MAX_LINES;

                size_t spaces = (span.column_start > 1) ? span.column_start - 1 : 0;

                output << BLUE << print_margin(start) << RED << "/ " << RESET << lines[0] << "\n";

                if (truncated) {
                    output << BLUE << print_margin(start + 1) << RED << "| " << RESET << lines[1] << "\n";
                    output << BLUE << print_margin(start + 2) << RED << "| " << RESET << lines[2] << "\n";
                    output << pipe_margin << RED << "| " << BLUE << "...\n" << RESET;
                    output << BLUE << print_margin(end - 1) << RED << "| " << RESET << lines[lines.size() - 2] << "\n";
                    output << BLUE << print_margin(end) << RED << "| " << RESET << lines[lines.size() - 1] << "\n";
                } else {
                    for (size_t i = 1; i < lines.size(); ++i) {
                        output << BLUE << print_margin(start + i) << RED << "| " << RESET << lines[i] << "\n";
                    }
                }

                output << pipe_margin << RED << "|___" << std::string(spaces, '_') << "^" << RESET;
            }
        }

        return output.str();
    }

    std::vector<std::string> DiagnosticFormatter::extract_lines(const std::string &source, size_t line_start,
                                                                size_t line_end) {
        std::istringstream stream(source);
        std::string line;
        std::vector<std::string> lines;
        size_t current = 1;

        while (std::getline(stream, line)) {
            if (current >= line_start && current <= line_end) {
                lines.push_back(line);
            }
            if (current == line_end) break;
            current++;
        }
        return lines;
    }
}
