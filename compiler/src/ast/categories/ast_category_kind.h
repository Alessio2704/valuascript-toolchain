#pragma once

#include <concepts>
#include "ast/core/ast_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    template <typename AllowedTuple>
    struct CategoryKind
    {
        AstKind value = AstKind::Program;

        constexpr CategoryKind() noexcept = default;
        constexpr explicit CategoryKind(AstKind k) noexcept : value(k) {}

        template <typename NodeT>
            requires (valuascript::shared::tuple_contains_type_v<NodeT, AllowedTuple>)
        static constexpr CategoryKind from() noexcept
        {
            return CategoryKind{NodeT::KIND};
        }

        template <typename NodeT>
            requires (valuascript::shared::tuple_contains_type_v<NodeT, AllowedTuple>)
        constexpr CategoryKind& operator=(NodeT) noexcept
        {
            value = NodeT::KIND;
            return *this;
        }

        [[nodiscard]] constexpr AstKind kind() const noexcept { return value; }
        [[nodiscard]] constexpr operator AstKind() const noexcept { return value; }
        [[nodiscard]] constexpr bool operator==(const CategoryKind& other) const noexcept = default;
        [[nodiscard]] constexpr bool operator==(AstKind other) const noexcept { return value == other; }
    };
}
