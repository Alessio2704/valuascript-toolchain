#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <concepts>
#include <type_traits>

#include "token/source_span.h"
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "ast_concepts.h"

namespace valuascript::compiler
{
    template <typename T>
    [[nodiscard]] inline bool ast_is_valid(const T& node) noexcept;

    template <typename T>
    [[nodiscard]] inline bool ast_is_valid(T* ptr) noexcept;

    inline bool ast_is_valid(const std::string& str) noexcept { return !str.empty(); }
    inline bool ast_is_valid(bool) noexcept { return true; }
    inline bool ast_is_valid(const SourceSpan& span) noexcept { return span.is_valid(); }
    inline bool ast_is_valid(const NodeName& name) noexcept { return name.is_valid(); }

    template <typename T>
    inline bool ast_is_valid(const std::optional<T>& opt) noexcept
    {
        return !opt.has_value() || ast_is_valid(*opt);
    }

    template <typename T>
    inline bool ast_is_valid(const std::unique_ptr<T>& ptr) noexcept
    {
        return ptr != nullptr && ast_is_valid(*ptr);
    }

    template <typename T>
    inline bool ast_is_valid(const std::vector<T>& vec) noexcept
    {
        for (const auto& item : vec)
        {
            if (!ast_is_valid(item)) return false;
        }
        return true;
    }

    template <typename T>
    inline bool ast_is_valid(T* ptr) noexcept
    {
        return ptr != nullptr && ast_is_valid(*ptr);
    }

    template <typename T>
    inline bool ast_is_valid(const T& node) noexcept
    {
        if constexpr (std::is_pointer_v<T>)
        {
            return node != nullptr && ast_is_valid(*node);
        }
        else if constexpr (requires { { node.is_valid() } -> std::same_as<bool>; })
        {
            return node.is_valid();
        }
        else
        {
            return true;
        }
    }
}
