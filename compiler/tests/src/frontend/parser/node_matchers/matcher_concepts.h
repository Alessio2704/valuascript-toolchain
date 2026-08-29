#pragma once

#include <concepts>
#include <type_traits>
#include "ast/ast.h"
#include "ast/core/ast_concepts.h"

namespace valuascript::compiler::test
{
    using valuascript::compiler::HasNodeType;
    using valuascript::compiler::IsCompatibleNodeCallable;
    using valuascript::compiler::AstNodeVisitor;

    template <typename F, typename NodeT>
    concept IsCompatibleNodeVerifier = IsCompatibleNodeCallable<F, NodeT>;

    template <typename M, typename NodeT>
    concept ASTMatcher = AstNodeVisitor<M, NodeT>;

    struct AnyMatcher
    {
        void operator()(AstNode*) const {}
        void operator()(Expression*) const {}
        void operator()(Statement*) const {}
        void operator()(TypeAnnotation*) const {}
        explicit operator bool() const { return false; }
    };

    template <typename M>
    concept ExprMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeCallable<M, Expression>;

    template <typename M>
    concept TypeNodeMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeCallable<M, TypeAnnotation>;

    template <typename M>
    concept StmtMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeCallable<M, Statement>;
}
