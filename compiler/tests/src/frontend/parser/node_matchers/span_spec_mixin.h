#pragma once

#include <optional>
#include <utility>
#include "token/source_span.h"

namespace valuascript::compiler::test
{
    template <typename Derived>
    struct SpanMixin
    {
        std::optional<SourceSpan> span = std::nullopt;

        Derived& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) &
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return static_cast<Derived&>(*this);
        }

        Derived with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) &&
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return std::move(static_cast<Derived&>(*this));
        }

        Derived& with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                           size_t start_offset, size_t length) &
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return static_cast<Derived&>(*this);
        }

        Derived with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                          size_t start_offset, size_t length) &&
        {
            span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return std::move(static_cast<Derived&>(*this));
        }

        Derived& with_span(const SourceSpan& s) &
        {
            span = s;
            return static_cast<Derived&>(*this);
        }

        Derived with_span(const SourceSpan& s) &&
        {
            span = s;
            return std::move(static_cast<Derived&>(*this));
        }
    };

    template <typename Derived>
    struct SpanSpecMixin : public SpanMixin<Derived>
    {
        std::optional<SourceSpan> name_span = std::nullopt;

        Derived& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) &
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return static_cast<Derived&>(*this);
        }

        Derived with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) &&
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return std::move(static_cast<Derived&>(*this));
        }

        Derived& with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                 size_t start_offset, size_t length) &
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return static_cast<Derived&>(*this);
        }

        Derived with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                               size_t start_offset, size_t length) &&
        {
            name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return std::move(static_cast<Derived&>(*this));
        }

        Derived& with_name_span(const SourceSpan& s) &
        {
            name_span = s;
            return static_cast<Derived&>(*this);
        }

        Derived with_name_span(const SourceSpan& s) &&
        {
            name_span = s;
            return std::move(static_cast<Derived&>(*this));
        }
    };
}
