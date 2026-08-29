#pragma once

#include <memory>
#include <vector>
#include "ast/core/ast_concepts.h"

namespace valuascript::compiler
{
    template <DirectlyValidatable T>
    [[nodiscard]] inline bool ast_is_valid(const T& node) noexcept
    {
        return node.is_valid();
    }

    template <DirectlyValidatable T>
    [[nodiscard]] inline bool ast_is_valid(const T* ptr) noexcept
    {
        return ptr != nullptr && ptr->is_valid();
    }

    template <DirectlyValidatable T>
    [[nodiscard]] inline bool ast_is_valid(const std::unique_ptr<T>& ptr) noexcept
    {
        return ptr != nullptr && ptr->is_valid();
    }

    template <DirectlyValidatable T>
    [[nodiscard]] inline bool ast_is_valid(const std::vector<T>& vec) noexcept
    {
        for (const auto& item : vec)
        {
            if (!item.is_valid()) return false;
        }
        return true;
    }

    template <DirectlyValidatable T>
    [[nodiscard]] inline bool ast_is_valid(const std::vector<std::unique_ptr<T>>& vec) noexcept
    {
        for (const auto& ptr : vec)
        {
            if (!ast_is_valid(ptr)) return false;
        }
        return true;
    }

    template <DirectlyValidatable T>
    [[nodiscard]] inline bool ast_is_valid(const std::vector<T*>& vec) noexcept
    {
        for (const auto& ptr : vec)
        {
            if (!ast_is_valid(ptr)) return false;
        }
        return true;
    }
}
