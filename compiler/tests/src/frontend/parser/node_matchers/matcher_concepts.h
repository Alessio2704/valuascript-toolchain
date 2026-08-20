#pragma once

#include <concepts>
#include <type_traits>
#include "ast/ast.h"

namespace valuascript::compiler::test
{
    struct AnyMatcher
    {
        void operator()(AstNode*) const {}
        void operator()(Expression*) const {}
        void operator()(Statement*) const {}
        void operator()(TypeAnnotation*) const {}
        explicit operator bool() const { return false; }
    };

    template <typename F, typename NodeT>
    concept HasNodeType = requires
    {
        typename std::decay_t<F>::node_type;
    };

    template <typename F, typename NodeT>
    concept IsCompatibleNodeVerifier =
        (HasNodeType<F, NodeT> && std::derived_from<typename std::decay_t<F>::node_type, NodeT>) ||
        (!HasNodeType<F, NodeT> && std::invocable<F, NodeT*>);

    template <typename M, typename NodeT>
    concept ASTMatcher = requires(const std::decay_t<M>& m, NodeT* node)
    {
        { m(node) };
    };

    template <typename M>
    concept ExprMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, Expression>;

    template <typename M>
    concept TypeNodeMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, TypeAnnotation>;

    template <typename M>
    concept StmtMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, Statement>;
}
