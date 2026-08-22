#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>

#include "ast_kind.h"

namespace valuascript::compiler
{
    class AstNode;
    class Expression;
    class Statement;
    class TypeAnnotation;

    template <typename T>
    concept AstElement = std::derived_from<std::decay_t<T>, AstNode>;

    template <typename T>
    concept ExpressionNode = std::derived_from<std::decay_t<T>, Expression>;

    template <typename T>
    concept StatementNode = std::derived_from<std::decay_t<T>, Statement>;

    template <typename T>
    concept TypeAnnNode = std::derived_from<std::decay_t<T>, TypeAnnotation>;

    template <typename T>
    concept ConcreteAstNode = AstElement<T> && requires { { T::KIND } -> std::same_as<const AstKind&>; };

    template <typename F>
    concept HasNodeType = requires
    {
        typename std::decay_t<F>::node_type;
    };

    template <typename F, typename NodeT>
    concept IsCompatibleNodeCallable =
        (HasNodeType<F> && std::derived_from<typename std::decay_t<F>::node_type, NodeT>) ||
        (!HasNodeType<F> && std::invocable<F, NodeT*>);

    template <typename M, typename NodeT>
    concept AstNodeVisitor = requires(const std::decay_t<M>& m, NodeT* node)
    {
        { m(node) };
    };

    template <typename T>
    concept DirectlyValidatable = requires(const T& node)
    {
        { node.is_valid() } -> std::same_as<bool>;
    };

    template <typename T>
    concept ValidatableAstNode = ConcreteAstNode<T> && DirectlyValidatable<T>;
}
