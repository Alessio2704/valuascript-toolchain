#pragma once

#include <string>
#include <vector>
#include "error_registry.h"

namespace valuascript::compiler::test
{
    class ErrorShifter
    {
    public:
        static std::vector<ParserExpectedError> shift_errors(const std::string& prefix_code,
                                                       const std::vector<ParserExpectedError>& original_errors)
        {
            size_t line_offset = 0;
            size_t col_offset = 0;

            for (char c : prefix_code)
            {
                if (c == '\n')
                {
                    line_offset++;
                    col_offset = 0;
                }
                else
                {
                    col_offset++;
                }
            }

            std::vector<ParserExpectedError> shifted;
            shifted.reserve(original_errors.size());

            for (const auto& err : original_errors)
            {
                ParserExpectedError shifted_err = err;

                if (!shifted_err.skip_span_check)
                {
                    if (shifted_err.line_start == 1)
                    {
                        shifted_err.column_start += col_offset;
                    }
                    shifted_err.line_start += line_offset;

                    if (shifted_err.line_end != 0)
                    {
                        if (shifted_err.line_end == 1)
                        {
                            shifted_err.column_end += col_offset;
                        }
                        shifted_err.line_end += line_offset;
                    }
                }

                shifted.push_back(shifted_err);
            }

            return shifted;
        }
    };
}
