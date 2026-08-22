#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <concepts>
#include <type_traits>

#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_concepts.h"
#include "ast/core/ast_node_registry.h"
#include "ast/core/ast_node_schema.h"

namespace valuascript::compiler
{
    [[nodiscard]] inline bool ast_is_disjoint(const AstNode* lhs, const AstNode* rhs) noexcept;
    [[nodiscard]] inline bool ast_is_disjoint(const AstNode& lhs, const AstNode& rhs) noexcept;

    template <HasAstNodeSchema T>
    [[nodiscard]] inline bool ast_node_is_disjoint(const T& lhs, const T& rhs) noexcept;

    template <typename T1, typename T2>
    requires (!std::is_pointer_v<T1> && !std::is_pointer_v<T2>)
    [[nodiscard]] inline bool ast_is_disjoint(const T1& lhs, const T2& rhs) noexcept;

    template <typename T1, typename T2>
    inline bool ast_is_disjoint(const T1* lhs, const T2* rhs) noexcept
    {
        if (!lhs || !rhs) return true;
        if (static_cast<const void*>(lhs) == static_cast<const void*>(rhs)) return false;
        if constexpr (std::derived_from<T1, AstNode> && std::derived_from<T2, AstNode>)
        {
            return ast_is_disjoint(static_cast<const AstNode*>(lhs), static_cast<const AstNode*>(rhs));
        }
        else
        {
            return true;
        }
    }

    template <typename T1, typename T2>
    inline bool ast_is_disjoint(const std::unique_ptr<T1>& lhs, const std::unique_ptr<T2>& rhs) noexcept
    {
        return ast_is_disjoint(lhs.get(), rhs.get());
    }

    template <typename T1, typename T2>
    inline bool ast_is_disjoint(const std::optional<T1>& lhs, const std::optional<T2>& rhs) noexcept
    {
        if (!lhs.has_value() || !rhs.has_value()) return true;
        return ast_is_disjoint(*lhs, *rhs);
    }

    template <typename T1, typename T2>
    inline bool ast_is_disjoint(const std::vector<T1>& lhs, const std::vector<T2>& rhs) noexcept
    {
        size_t common_size = std::min(lhs.size(), rhs.size());
        for (size_t i = 0; i < common_size; ++i)
        {
            if (!ast_is_disjoint(lhs[i], rhs[i])) return false;
        }
        return true;
    }

    template <typename T1, typename T2>
    requires (!std::is_pointer_v<T1> && !std::is_pointer_v<T2>)
    inline bool ast_is_disjoint(const T1& lhs, const T2& rhs) noexcept
    {
        if constexpr (std::derived_from<T1, AstNode> && std::derived_from<T2, AstNode>)
        {
            return ast_is_disjoint(&lhs, &rhs);
        }
        else if constexpr (std::same_as<T1, T2> && HasAstNodeSchema<T1>)
        {
            return ast_node_is_disjoint(lhs, rhs);
        }
        else
        {
            return true;
        }
    }

    template <HasAstNodeSchema T>
    inline bool ast_node_is_disjoint(const T& lhs, const T& rhs) noexcept
    {
        bool disjoint = true;
        for_each_ast_member_pair(lhs, rhs, [&](const auto& prop_l, const auto& prop_r) {
            if (!disjoint) return;
            if (!ast_is_disjoint(prop_l, prop_r))
            {
                disjoint = false;
            }
        });
        return disjoint;
    }

    inline bool ast_is_disjoint(const AstNode* lhs, const AstNode* rhs) noexcept
    {
        if (!lhs || !rhs) return true;
        if (lhs == rhs) return false;
        if (lhs->kind != rhs->kind) return true;

        bool disjoint = true;
        NodeDispatcher<AllAstNodeTypes>::dispatch(lhs->kind, [&]<typename T>() {
            disjoint = ast_node_is_disjoint(*static_cast<const T*>(lhs), *static_cast<const T*>(rhs));
        });
        return disjoint;
    }

    inline bool ast_is_disjoint(const AstNode& lhs, const AstNode& rhs) noexcept
    {
        return ast_is_disjoint(&lhs, &rhs);
    }

    template <typename T>
    concept DisjointableAstNode = ConcreteAstNode<T> && requires(const T& node)
    {
        { ast_is_disjoint(node, node) } -> std::same_as<bool>;
    };
}
