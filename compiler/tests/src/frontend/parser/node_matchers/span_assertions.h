#pragma once

#include <gtest/gtest.h>
#include <source_location>
#include "ast/ast.h"

namespace valuascript::compiler::test
{
    inline void AssertSpanMatch(const SourceSpan& actual, const SourceSpan& expected,
                                std::source_location loc = std::source_location::current())
    {
        if (actual.line_start != expected.line_start ||
            actual.column_start != expected.column_start ||
            actual.line_end != expected.line_end ||
            actual.column_end != expected.column_end)
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Span line/column mismatch:\n"
                << "  Expected: line " << expected.line_start << ":" << expected.column_start
                << " -> line " << expected.line_end << ":" << expected.column_end << "\n"
                << "  Actual:   line " << actual.line_start << ":" << actual.column_start
                << " -> line " << actual.line_end << ":" << actual.column_end;
        }

        if (expected.start_offset != 0 || expected.length != 0)
        {
            if (actual.start_offset != expected.start_offset || actual.length != expected.length)
            {
                ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                    << "Span offset/length mismatch:\n"
                    << "  Expected: offset " << expected.start_offset << ", length " << expected.length
                    << ", end_offset " << expected.end_offset() << "\n"
                    << "  Actual:   offset " << actual.start_offset << ", length " << actual.length
                    << ", end_offset " << actual.end_offset();
            }
        }

        if (!expected.path().empty())
        {
            if (actual.path() != expected.path())
            {
                ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                    << "Span file_path mismatch:\n"
                    << "  Expected: " << expected.path() << "\n"
                    << "  Actual:   " << actual.path();
            }
        }
    }
}
