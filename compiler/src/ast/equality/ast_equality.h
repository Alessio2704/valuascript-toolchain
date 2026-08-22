#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <concepts>
#include <type_traits>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_concepts.h"
#include "ast/core/ast_node_registry.h"
#include "ast/core/ast_node_schema.h"

namespace valuascript::compiler
{
    [[nodiscard]] inline bool ast_equals(const AstNode* lhs, const AstNode* rhs) noexcept;
    [[nodiscard]] inline bool ast_equals(const AstNode& lhs, const AstNode& rhs) noexcept;

    template <HasAstNodeSchema T>
    [[nodiscard]] inline bool ast_node_equals(const T& lhs, const T& rhs) noexcept;

    template <typename T1, typename T2>
    requires (!std::is_pointer_v<T1> && !std::is_pointer_v<T2>)
    [[nodiscard]] inline bool ast_equals(const T1& lhs, const T2& rhs) noexcept;

    inline bool ast_equals(const std::string& lhs, const std::string& rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(bool lhs, bool rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(TokenType lhs, TokenType rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(AstKind lhs, AstKind rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(const SourceSpan& lhs, const SourceSpan& rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(const NodeName& lhs, const NodeName& rhs) noexcept { return lhs == rhs; }

    template <typename T1, typename T2>
    inline bool ast_equals(const T1* lhs, const T2* rhs) noexcept
    {
        if (static_cast<const void*>(lhs) == static_cast<const void*>(rhs)) return true;
        if (!lhs || !rhs) return false;
        if constexpr (std::derived_from<T1, AstNode> && std::derived_from<T2, AstNode>)
        {
            return ast_equals(static_cast<const AstNode*>(lhs), static_cast<const AstNode*>(rhs));
        }
        else if constexpr (std::equality_comparable_with<T1, T2>)
        {
            return *lhs == *rhs;
        }
        else
        {
            return false;
        }
    }

    template <typename T1, typename T2>
    inline bool ast_equals(const std::unique_ptr<T1>& lhs, const std::unique_ptr<T2>& rhs) noexcept
    {
        return ast_equals(lhs.get(), rhs.get());
    }

    template <typename T1, typename T2>
    inline bool ast_equals(const std::optional<T1>& lhs, const std::optional<T2>& rhs) noexcept
    {
        if (lhs.has_value() != rhs.has_value()) return false;
        if (!lhs.has_value()) return true;
        return ast_equals(*lhs, *rhs);
    }

    template <typename T1, typename T2>
    inline bool ast_equals(const std::vector<T1>& lhs, const std::vector<T2>& rhs) noexcept
    {
        if (lhs.size() != rhs.size()) return false;
        for (size_t i = 0; i < lhs.size(); ++i)
        {
            if (!ast_equals(lhs[i], rhs[i])) return false;
        }
        return true;
    }

    template <typename T1, typename T2>
    requires (!std::is_pointer_v<T1> && !std::is_pointer_v<T2>)
    inline bool ast_equals(const T1& lhs, const T2& rhs) noexcept
    {
        if constexpr (std::derived_from<T1, AstNode> && std::derived_from<T2, AstNode>)
        {
            return ast_equals(&lhs, &rhs);
        }
        else if constexpr (std::same_as<T1, T2> && HasAstNodeSchema<T1>)
        {
            return ast_node_equals(lhs, rhs);
        }
        else if constexpr (std::equality_comparable_with<T1, T2>)
        {
            return lhs == rhs;
        }
        else
        {
            return false;
        }
    }

    template <HasAstNodeSchema T>
    inline bool ast_node_equals(const T& lhs, const T& rhs) noexcept
    {
        bool matches = true;
        for_each_ast_member_pair(lhs, rhs, [&](const auto& prop_l, const auto& prop_r) {
            if (!matches) return;
            if (!ast_equals(prop_l, prop_r))
            {
                matches = false;
            }
        });
        return matches;
    }

    inline bool ast_equals(const AstNode* lhs, const AstNode* rhs) noexcept
    {
        if (lhs == rhs) return true;
        if (!lhs || !rhs) return false;
        if (lhs->kind != rhs->kind) return false;
        if (lhs->span != rhs->span) return false;

        bool equal = false;
        NodeDispatcher<AllAstNodeTypes>::dispatch(lhs->kind, [&]<typename T>() {
            equal = ast_node_equals(*static_cast<const T*>(lhs), *static_cast<const T*>(rhs));
        });
        return equal;
    }

    inline bool ast_equals(const AstNode& lhs, const AstNode& rhs) noexcept
    {
        return ast_equals(&lhs, &rhs);
    }

    template <typename T>
    concept ComparableAstNode = ConcreteAstNode<T> && requires(const T& node)
    {
        { ast_equals(node, node) } -> std::same_as<bool>;
    };
}
