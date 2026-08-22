#pragma once

#include <memory>
#include <type_traits>
#include <string_view>
#include <cassert>

#include "token/source_span.h"
#include "ast_kind.h"
#include "ast_concepts.h"

namespace valuascript::compiler
{
    template <AstElement T, AstElement NodeT>
    [[nodiscard]] inline auto* ast_cast(NodeT* node) noexcept
    {
        using ReturnType = std::conditional_t<std::is_const_v<NodeT>, const T, T>;
        if (!node) [[unlikely]] return static_cast<ReturnType*>(nullptr);
        if constexpr (requires { T::KIND; })
        {
            if (node->kind == T::KIND) [[likely]] return static_cast<ReturnType*>(node);
            return static_cast<ReturnType*>(nullptr);
        }
        else
        {
            return dynamic_cast<ReturnType*>(node);
        }
    }

    template <AstElement T, AstElement NodeT>
    [[nodiscard]] inline bool is_a(NodeT* node) noexcept
    {
        return ast_cast<T>(node) != nullptr;
    }

    template <AstElement T, AstElement NodeT>
    [[nodiscard]] inline auto& as(NodeT& node) noexcept
    {
        auto* casted = ast_cast<T>(&node);
        assert(casted != nullptr && "Invalid AST node cast with as<T>()");
        return *casted;
    }

    template <AstElement T, AstElement Base>
    [[nodiscard]] inline std::unique_ptr<T> ast_cast_unique(std::unique_ptr<Base> ptr) noexcept
    {
        if (!ptr) return nullptr;
        if (ast_cast<T>(ptr.get()))
        {
            return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
        }
        return nullptr;
    }

    template <typename T>
    [[nodiscard]] constexpr std::string_view get_node_name(const T& elem) noexcept
    {
        if constexpr (requires { elem.name.value; }) return elem.name.value;
        else if constexpr (requires { elem.property_name.value; }) return elem.property_name.value;
        else if constexpr (requires { elem.key.value; }) return elem.key.value;
        else if constexpr (requires { elem.path.value; }) return elem.path.value;
        else if constexpr (requires { elem.name; }) return elem.name;
        else return "";
    }

    template <typename T>
    [[nodiscard]] constexpr const valuascript::shared::SourceSpan* get_node_name_span(const T& elem) noexcept
    {
        if constexpr (requires { elem.name.span; }) return &elem.name.span;
        else if constexpr (requires { elem.path.span; }) return &elem.path.span;
        else if constexpr (requires { elem.property_name.span; }) return &elem.property_name.span;
        else return nullptr;
    }

    template <AstElement T>
    [[nodiscard]] constexpr std::string_view get_ast_node_name() noexcept
    {
        using DecayedT = std::decay_t<T>;
        if constexpr (requires { DecayedT::KIND; })
        {
            return to_string(DecayedT::KIND);
        }
        else if constexpr (std::same_as<DecayedT, Expression>)
        {
            return "Expression";
        }
        else if constexpr (std::same_as<DecayedT, Statement>)
        {
            return "Statement";
        }
        else if constexpr (std::same_as<DecayedT, TypeAnnotation>)
        {
            return "TypeAnnotation";
        }
        else if constexpr (std::same_as<DecayedT, AstNode>)
        {
            return "AstNode";
        }
        else
        {
            return "Unknown";
        }
    }

    [[nodiscard]] inline std::string_view get_ast_node_name(const AstNode& node) noexcept
    {
        return to_string(node.kind);
    }
}
