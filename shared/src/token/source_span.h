#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <type_traits>
#include <cstddef>
#include <utility>

namespace valuascript::shared
{
    struct SourceSpan
    {
        size_t line_start = 0;
        size_t column_start = 0;
        size_t line_end = 0;
        size_t column_end = 0;
        std::shared_ptr<const std::string> file_path = nullptr;
        size_t start_offset = 0;
        size_t length = 0;

        SourceSpan& operator=(std::string fp)
        {
            file_path = std::make_shared<const std::string>(std::move(fp));
            return *this;
        }

        SourceSpan& operator=(const char* fp)
        {
            file_path = std::make_shared<const std::string>(fp ? fp : "");
            return *this;
        }

        [[nodiscard]] std::string_view path() const noexcept
        {
            return file_path ? *file_path : std::string_view{};
        }

        [[nodiscard]] size_t end_offset() const noexcept
        {
            return start_offset + length;
        }

        [[nodiscard]] constexpr bool is_valid() const noexcept
        {
            return line_start > 0 && column_start > 0 && line_end >= line_start &&
                   (line_end > line_start || column_end >= column_start);
        }

        [[nodiscard]] constexpr bool contains(size_t line, size_t column) const noexcept
        {
            if (line < line_start || line > line_end) return false;
            if (line == line_start && column < column_start) return false;
            if (line == line_end && column > column_end) return false;
            return true;
        }

        [[nodiscard]] constexpr bool contains_offset(size_t offset) const noexcept
        {
            return offset >= start_offset && offset < start_offset + length;
        }

        [[nodiscard]] constexpr bool starts_before(const SourceSpan& other) const noexcept
        {
            if (line_start != other.line_start) return line_start < other.line_start;
            return column_start < other.column_start;
        }

        [[nodiscard]] constexpr bool starts_after(const SourceSpan& other) const noexcept
        {
            if (line_start != other.line_start) return line_start > other.line_start;
            return column_start > other.column_start;
        }

        [[nodiscard]] constexpr bool operator<(const SourceSpan& other) const noexcept
        {
            return starts_before(other);
        }

        [[nodiscard]] constexpr bool matches_lines_columns(const SourceSpan& other) const noexcept
        {
            return line_start == other.line_start && column_start == other.column_start &&
                   line_end == other.line_end && column_end == other.column_end;
        }

        [[nodiscard]] constexpr bool matches_offsets(const SourceSpan& other) const noexcept
        {
            return start_offset == other.start_offset && length == other.length;
        }

        [[nodiscard]] bool matches_file_path(const SourceSpan& other) const noexcept
        {
            return file_path == other.file_path || (file_path && other.file_path && *file_path == *other.file_path);
        }

        [[nodiscard]] bool matches(const SourceSpan& pattern) const noexcept
        {
            if (!matches_lines_columns(pattern)) return false;
            if ((pattern.start_offset != 0 || pattern.length != 0) && !matches_offsets(pattern)) return false;
            if (!pattern.path().empty() && path() != pattern.path()) return false;
            return true;
        }

        [[nodiscard]] bool operator==(const SourceSpan& other) const
        {
            return matches_lines_columns(other) && matches_offsets(other) && matches_file_path(other);
        }
    };

    static_assert(std::is_aggregate_v<SourceSpan>, "SourceSpan must be a C++20 aggregate struct");
}

namespace valuascript::compiler
{
    using SourceSpan = valuascript::shared::SourceSpan;
}
