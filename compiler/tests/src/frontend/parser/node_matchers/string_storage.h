#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <compare>
#include "ast/ast.h"

namespace valuascript::compiler::test
{
    struct StringStorage
    {
        std::variant<std::string_view, std::string> data_;

        constexpr StringStorage() : data_(std::in_place_index<0>, std::string_view{})
        {
        }

        constexpr StringStorage(const char* s) : data_(std::in_place_index<0>, std::string_view(s ? s : ""))
        {
        }

        constexpr StringStorage(std::string_view sv) : data_(std::in_place_index<0>, sv)
        {
        }

        StringStorage(std::string s) : data_(std::move(s))
        {
        }

        [[nodiscard]] operator std::string_view() const { return get(); }

        [[nodiscard]] std::string_view get() const
        {
            if (auto* sv = std::get_if<std::string_view>(&data_)) [[likely]] return *sv;
            return std::get<std::string>(data_);
        }

        [[nodiscard]] const char* data() const { return get().data(); }
        [[nodiscard]] size_t size() const { return get().size(); }
        [[nodiscard]] bool empty() const { return get().empty(); }

        [[nodiscard]] friend bool operator==(const StringStorage& lhs, std::string_view rhs)
        {
            return lhs.get() == rhs;
        }

        [[nodiscard]] friend bool operator==(const StringStorage& lhs, const NodeName& rhs)
        {
            return lhs.get() == rhs.value;
        }

        [[nodiscard]] friend std::strong_ordering operator<=>(const StringStorage& lhs, std::string_view rhs)
        {
            return lhs.get() <=> rhs;
        }
    };
}
