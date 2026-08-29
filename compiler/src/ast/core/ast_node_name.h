#pragma once

#include <string>
#include <string_view>
#include <ostream>
#include <utility>

#include "token/source_span.h"

namespace valuascript::compiler
{
    struct NodeName
    {
        std::string value;
        valuascript::shared::SourceSpan span = {};

        NodeName() = default;

        NodeName(std::string_view val, valuascript::shared::SourceSpan sp = {})
            : value(val), span(std::move(sp))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept
        {
            return !value.empty() && span.is_valid();
        }

        [[nodiscard]] const std::string& str() const noexcept { return value; }
        [[nodiscard]] const char* c_str() const noexcept { return value.c_str(); }
        [[nodiscard]] bool empty() const noexcept { return value.empty(); }
        [[nodiscard]] size_t length() const noexcept { return value.length(); }

        operator std::string_view() const noexcept { return value; }
        operator const std::string&() const noexcept { return value; }
        bool operator==(const NodeName& other) const = default;
        bool operator==(std::string_view other) const noexcept { return value == other; }
        bool operator==(const char* other) const noexcept { return value == other; }
        bool operator==(const std::string& other) const noexcept { return value == other; }

        friend std::string operator+(std::string_view lhs, const NodeName& rhs)
        {
            std::string result(lhs);
            result += rhs.value;
            return result;
        }

        friend std::string operator+(const NodeName& lhs, std::string_view rhs)
        {
            std::string result(lhs.value);
            result += rhs;
            return result;
        }

        friend std::string operator+(const std::string& lhs, const NodeName& rhs)
        {
            return lhs + rhs.value;
        }

        friend std::string operator+(const NodeName& lhs, const std::string& rhs)
        {
            return lhs.value + rhs;
        }

        friend std::string operator+(const char* lhs, const NodeName& rhs)
        {
            return std::string(lhs) + rhs.value;
        }

        friend std::string operator+(const NodeName& lhs, const char* rhs)
        {
            return lhs.value + rhs;
        }

        friend std::ostream& operator<<(std::ostream& os, const NodeName& name)
        {
            return os << name.value;
        }
    };
}
