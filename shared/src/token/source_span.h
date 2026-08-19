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

        [[nodiscard]] bool operator==(const SourceSpan& other) const
        {
            return line_start == other.line_start && column_start == other.column_start &&
                   line_end == other.line_end && column_end == other.column_end &&
                   start_offset == other.start_offset && length == other.length &&
                   (file_path == other.file_path || (file_path && other.file_path && *file_path == *other.file_path));
        }
    };

    static_assert(std::is_aggregate_v<SourceSpan>, "SourceSpan must be a C++20 aggregate struct");
}

namespace valuascript::compiler
{
    using SourceSpan = valuascript::shared::SourceSpan;
}
