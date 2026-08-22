#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <concepts>
#include <type_traits>

#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "ast_concepts.h"
#include "ast_node_registry.h"
#include "ast_node_schema.h"

namespace valuascript::compiler
{
    template <typename T1, typename T2>
    [[nodiscard]] bool ast_is_disjoint(const T1& lhs, const T2& rhs) noexcept;

    [[nodiscard]] inline bool ast_is_disjoint(const AstNode* lhs, const AstNode* rhs) noexcept;
    [[nodiscard]] inline bool ast_is_disjoint(const AstNode& lhs, const AstNode& rhs) noexcept;

    template <HasAstNodeSchema T>
    [[nodiscard]] inline bool ast_node_is_disjoint(const T& lhs, const T& rhs) noexcept
    {
        bool disjoint = true;
        for_each_ast_member_pair(lhs, rhs, [&]<typename A, typename B>(const A& a, const B& b) {
            if (!disjoint) return;
            if (!ast_is_disjoint(a, b))
            {
                disjoint = false;
            }
        });
        return disjoint;
    }

    template <typename T1, typename T2>
    [[nodiscard]] inline bool ast_is_disjoint(const std::optional<T1>& lhs, const std::optional<T2>& rhs) noexcept
    {
        if (!lhs.has_value() || !rhs.has_value()) return true;
        return ast_is_disjoint(*lhs, *rhs);
    }

    template <typename T1, typename T2>
    [[nodiscard]] inline bool ast_is_disjoint(const std::unique_ptr<T1>& lhs,
                                              const std::unique_ptr<T2>& rhs) noexcept
    {
        if (!lhs || !rhs) return true;
        if (static_cast<const void*>(lhs.get()) == static_cast<const void*>(rhs.get())) return false;
        return ast_is_disjoint(*lhs, *rhs);
    }

    template <typename T1, typename T2>
    [[nodiscard]] inline bool ast_is_disjoint(const std::vector<T1>& lhs,
                                              const std::vector<T2>& rhs) noexcept
    {
        size_t count = std::min(lhs.size(), rhs.size());
        for (size_t i = 0; i < count; ++i)
        {
            if (!ast_is_disjoint(lhs[i], rhs[i])) return false;
        }
        return true;
    }

    template <typename T1, typename T2>
    [[nodiscard]] inline bool ast_is_disjoint(const T1& lhs, const T2& rhs) noexcept
    {
        if constexpr (std::is_pointer_v<std::decay_t<T1>> && std::is_pointer_v<std::decay_t<T2>>)
        {
            using P1 = std::remove_pointer_t<std::decay_t<T1>>;
            using P2 = std::remove_pointer_t<std::decay_t<T2>>;
            if constexpr (AstElement<P1> && AstElement<P2>)
            {
                if (!lhs || !rhs) return true;
                if (static_cast<const void*>(lhs) == static_cast<const void*>(rhs)) return false;
                return ast_is_disjoint(static_cast<const AstNode&>(*lhs), static_cast<const AstNode&>(*rhs));
            }
            else
            {
                return true;
            }
        }
        else if constexpr (std::same_as<T1, T2> && HasAstNodeSchema<T1>)
        {
            return ast_node_is_disjoint(lhs, rhs);
        }
        else if constexpr (AstElement<T1> && AstElement<T2>)
        {
            return ast_is_disjoint(static_cast<const AstNode&>(lhs), static_cast<const AstNode&>(rhs));
        }
        else
        {
            return true;
        }
    }

    template <AstElement T1, AstElement T2>
    inline bool ast_is_disjoint(const T1* lhs, const T2* rhs) noexcept
    {
        if (!lhs || !rhs) return true;
        if (static_cast<const void*>(lhs) == static_cast<const void*>(rhs)) return false;
        return ast_is_disjoint(static_cast<const AstNode&>(*lhs), static_cast<const AstNode&>(*rhs));
    }

    inline bool ast_is_disjoint(const AstNode* lhs, const AstNode* rhs) noexcept
    {
        if (!lhs || !rhs) return true;
        if (lhs == rhs) return false;
        return ast_is_disjoint(*lhs, *rhs);
    }

    inline bool ast_is_disjoint(const AstNode& lhs, const AstNode& rhs) noexcept
    {
        if (&lhs == &rhs) return false;
        if (lhs.kind != rhs.kind) return true;

        bool result = true;
        bool dispatched = NodeDispatcher<AllAstNodeTypes>::dispatch(lhs.kind, [&]<typename T>() {
            if constexpr (HasAstNodeSchema<T>)
            {
                result = ast_node_is_disjoint(static_cast<const T&>(lhs), static_cast<const T&>(rhs));
            }
            else
            {
                result = true;
            }
        });
        return !dispatched || result;
    }
}
