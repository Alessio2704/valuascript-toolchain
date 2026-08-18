#include <string>
#include <string_view>
#include <vector>
#include <format>
#include <iostream>
#include "diagnostic_formatter.h"

namespace valuascript::compiler
{
    void DiagnosticFormatter::print_errors(const std::vector<ValuaScriptException>& errors,
                                           const SourceManager& source_manager)
    {
        for (const auto& err : errors)
        {
            const auto& loc = err.get_span();
            std::string_view source_code;

            if (auto source_opt = source_manager.get_source(loc.path()); source_opt.has_value())
            {
                source_code = source_opt.value();
            }

            std::cout << format_error(err, source_code) << "\n\n";
        }
    }

    std::string DiagnosticFormatter::format_error(const ValuaScriptException& err, std::string_view source_code)
    {
        std::string output;
        output.reserve(512);

        const auto& span = err.get_span();

        std::format_to(std::back_inserter(output), "{}{}error[E{}]: {}{}{}\n",
                       RED, BOLD, err.get_error_number(), RESET, BOLD, err.what());
        std::format_to(std::back_inserter(output), "{}  --> {}{}:{}:{}\n",
                       BLUE, RESET, span.path(), span.line_start, span.column_start);

        size_t start = span.line_start;
        size_t end = (span.line_end > 0) ? span.line_end : start;
        if (end < start) end = start;

        std::vector<std::string_view> lines = extract_lines(source_code, start, end);

        auto padding = std::to_string(end).length();

        auto print_margin = [&](size_t line_num)
        {
            return std::format(" {:>{}} | ", line_num, padding);
        };

        std::string empty_margin(padding + 4, ' ');
        std::string pipe_margin(padding + 2, ' ');

        if (start == end)
        {
            std::string_view target_line = lines.empty() ? "<source line not available>" : lines[0];
            std::format_to(std::back_inserter(output), "{}{}{}{}\n",
                           BLUE, print_margin(start), RESET, target_line);

            size_t spaces = (span.column_start > 1) ? span.column_start - 1 : 0;
            std::string indent(spaces, ' ');

            size_t squiggle_length = 1;
            if (span.column_end > span.column_start)
            {
                squiggle_length = span.column_end - span.column_start;
            }

            std::string squiggles = "^";
            if (squiggle_length > 1)
            {
                squiggles += std::string(squiggle_length - 1, '~');
            }

            std::format_to(std::back_inserter(output), "{}{}{}{}{}",
                           empty_margin, indent, RED, squiggles, RESET);
        }
        else
        {
            if (lines.empty())
            {
                std::format_to(std::back_inserter(output), "{}{}{}<source line not available>\n",
                               BLUE, print_margin(start), RESET);
            }
            else
            {
                constexpr size_t MAX_LINES = 6;
                bool truncated = lines.size() > MAX_LINES;

                size_t spaces = (span.column_start > 1) ? span.column_start - 1 : 0;

                std::format_to(std::back_inserter(output), "{}{}{}/ {}{}\n",
                               BLUE, print_margin(start), RED, RESET, lines[0]);

                if (truncated)
                {
                    std::format_to(std::back_inserter(output), "{}{}{}| {}{}\n",
                                   BLUE, print_margin(start + 1), RED, RESET, lines[1]);
                    std::format_to(std::back_inserter(output), "{}{}{}| {}{}\n",
                                   BLUE, print_margin(start + 2), RED, RESET, lines[2]);
                    std::format_to(std::back_inserter(output), "{}{}| {}{}...\n",
                                   pipe_margin, RED, BLUE, RESET);
                    std::format_to(std::back_inserter(output), "{}{}{}| {}{}\n",
                                   BLUE, print_margin(end - 1), RED, RESET, lines[lines.size() - 2]);
                    std::format_to(std::back_inserter(output), "{}{}{}| {}{}\n",
                                   BLUE, print_margin(end), RED, RESET, lines[lines.size() - 1]);
                }
                else
                {
                    for (size_t i = 1; i < lines.size(); ++i)
                    {
                        std::format_to(std::back_inserter(output), "{}{}{}| {}{}\n",
                                       BLUE, print_margin(start + i), RED, RESET, lines[i]);
                    }
                }

                std::format_to(std::back_inserter(output), "{}{}|___{}^{}",
                               pipe_margin, RED, std::string(spaces, '_'), RESET);
            }
        }

        return output;
    }

    std::vector<std::string_view> DiagnosticFormatter::extract_lines(std::string_view source, size_t line_start,
                                                                     size_t line_end)
    {
        std::vector<std::string_view> lines;
        if (source.empty() || line_start == 0) return lines;

        size_t current_line = 1;
        size_t line_begin = 0;

        for (size_t i = 0; i <= source.size(); ++i)
        {
            if (i == source.size() || source[i] == '\n')
            {
                if (current_line >= line_start && current_line <= line_end)
                {
                    std::string_view line_view = source.substr(line_begin, i - line_begin);
                    if (!line_view.empty() && line_view.back() == '\r')
                    {
                        line_view.remove_suffix(1);
                    }
                    lines.push_back(line_view);
                }
                if (current_line == line_end) break;
                current_line++;
                line_begin = i + 1;
            }
        }
        return lines;
    }
}
