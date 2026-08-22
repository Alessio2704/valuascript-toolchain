#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <concepts>
#include <type_traits>

#include "token/source_span.h"
#include "token/token_type.h"
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
    [[nodiscard]] bool ast_equals(const AstNode* lhs, const AstNode* rhs) noexcept;
    [[nodiscard]] bool ast_equals(const AstNode& lhs, const AstNode& rhs) noexcept;

    inline bool ast_equals(const std::string& lhs, const std::string& rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(bool lhs, bool rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(TokenType lhs, TokenType rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(AstKind lhs, AstKind rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(const SourceSpan& lhs, const SourceSpan& rhs) noexcept { return lhs == rhs; }
    inline bool ast_equals(const NodeName& lhs, const NodeName& rhs) noexcept { return lhs == rhs; }

    template <typename T1, typename T2>
    inline bool ast_equals(const std::optional<T1>& lhs, const std::optional<T2>& rhs) noexcept
    {
        if (lhs.has_value() != rhs.has_value()) return false;
        if (!lhs.has_value()) return true;
        return ast_equals(*lhs, *rhs);
    }

    template <typename T1, typename T2>
    inline bool ast_equals(const std::unique_ptr<T1>& lhs, const std::unique_ptr<T2>& rhs) noexcept
    {
        if (!lhs && !rhs) return true;
        if (!lhs || !rhs) return false;
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

    template <HasAstNodeSchema T>
    [[nodiscard]] inline bool ast_node_equals(const T& lhs, const T& rhs) noexcept
    {
        if (lhs.kind != rhs.kind) return false;
        if (lhs.span != rhs.span) return false;
        bool equal = true;
        for_each_ast_member_pair(lhs, rhs, [&]<typename A, typename B>(const A& a, const B& b) {
            if (!equal) return;
            if (!ast_equals(a, b))
            {
                equal = false;
            }
        });
        return equal;
    }

    template <typename T1, typename T2>
    inline bool ast_equals(const T1& lhs, const T2& rhs) noexcept
    {
        if constexpr (std::is_pointer_v<std::decay_t<T1>> && std::is_pointer_v<std::decay_t<T2>>)
        {
            using P1 = std::remove_pointer_t<std::decay_t<T1>>;
            using P2 = std::remove_pointer_t<std::decay_t<T2>>;
            if constexpr (AstElement<P1> && AstElement<P2>)
            {
                if (static_cast<const void*>(lhs) == static_cast<const void*>(rhs)) return true;
                if (!lhs || !rhs) return false;
                return ast_equals(static_cast<const AstNode&>(*lhs), static_cast<const AstNode&>(*rhs));
            }
            else
            {
                return lhs == rhs;
            }
        }
        else if constexpr (std::same_as<T1, T2> && HasAstNodeSchema<T1>)
        {
            return ast_node_equals(lhs, rhs);
        }
        else if constexpr (AstElement<T1> && AstElement<T2>)
        {
            return ast_equals(static_cast<const AstNode&>(lhs), static_cast<const AstNode&>(rhs));
        }
        else if constexpr (requires { lhs == rhs; })
        {
            return lhs == rhs;
        }
        else
        {
            return false;
        }
    }

    template <AstElement T1, AstElement T2>
    inline bool ast_equals(const T1* lhs, const T2* rhs) noexcept
    {
        if (static_cast<const void*>(lhs) == static_cast<const void*>(rhs)) return true;
        if (!lhs || !rhs) return false;
        return ast_equals(static_cast<const AstNode&>(*lhs), static_cast<const AstNode&>(*rhs));
    }

    inline bool ast_equals(const AstNode* lhs, const AstNode* rhs) noexcept
    {
        if (lhs == rhs) return true;
        if (!lhs || !rhs) return false;
        return ast_equals(*lhs, *rhs);
    }

    inline bool ast_equals(const AstNode& lhs, const AstNode& rhs) noexcept
    {
        if (&lhs == &rhs) return true;
        if (lhs.kind != rhs.kind) return false;
        if (lhs.span != rhs.span) return false;

        bool result = false;
        bool dispatched = NodeDispatcher<AllAstNodeTypes>::dispatch(lhs.kind, [&]<typename T>() {
            if constexpr (HasAstNodeSchema<T>)
            {
                result = ast_node_equals(static_cast<const T&>(lhs), static_cast<const T&>(rhs));
            }
            else
            {
                result = true;
            }
        });
        return dispatched && result;
    }
}
