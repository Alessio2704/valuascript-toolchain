#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <type_traits>
#include <concepts>

#include "ast/core/ast_core.h"
#include "ast/core/ast_optional_field.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    struct is_unique_ptr : std::false_type
    {
    };

    template <typename T, typename Deleter>
    struct is_unique_ptr<std::unique_ptr<T, Deleter>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_unique_ptr_v = is_unique_ptr<std::remove_cvref_t<T>>::value;

    template <typename T>
    struct is_std_vector : std::false_type
    {
    };

    template <typename T, typename Alloc>
    struct is_std_vector<std::vector<T, Alloc>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_std_vector_v = is_std_vector<std::remove_cvref_t<T>>::value;

    template <typename T>
    struct is_optional_ast_field : std::false_type
    {
    };

    template <typename T>
    struct is_optional_ast_field<OptionalAstField<T>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_optional_ast_field_v = is_optional_ast_field<std::remove_cvref_t<T>>::value;

    template <typename T>
    struct is_std_optional : std::false_type
    {
    };

    template <typename T>
    struct is_std_optional<std::optional<T>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_std_optional_v = is_std_optional<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept IsUniquePtrOfAstNode = requires
    {
        typename std::remove_cvref_t<T>::element_type;
    } && is_unique_ptr_v<T> && std::derived_from<typename std::remove_cvref_t<T>::element_type, AstNode>;

    template <typename T>
    concept IsVectorOfUniquePtrOfAstNode = is_std_vector_v<T> && requires
    {
        typename std::remove_cvref_t<T>::value_type::element_type;
    } && is_unique_ptr_v<typename std::remove_cvref_t<T>::value_type> && std::derived_from<
        typename std::remove_cvref_t<T>::value_type::element_type, AstNode>;

    template <typename T>
    concept IsInnerAstNode = valuascript::shared::tuple_contains_type_v<std::remove_cvref_t<T>, AllInnerNodeTypes>;

    template <typename T>
    concept IsVectorOfInnerAstNode = is_std_vector_v<T> && valuascript::shared::tuple_contains_type_v<
        typename std::remove_cvref_t<T>::value_type, AllInnerNodeTypes>;

    template <typename T>
    concept IsOptionalAstFieldOfAstNode = is_optional_ast_field_v<T> && (
        (requires { typename std::remove_cvref_t<T>::value_type::element_type; } &&
            is_unique_ptr_v<typename std::remove_cvref_t<T>::value_type> &&
            std::derived_from<typename std::remove_cvref_t<T>::value_type::element_type, AstNode>) ||
        IsInnerAstNode<typename std::remove_cvref_t<T>::value_type>
    );

    template <typename T>
    concept IsStdOptionalOfAstNode = is_std_optional_v<T> && (
        (requires { typename std::remove_cvref_t<T>::value_type::element_type; } &&
            is_unique_ptr_v<typename std::remove_cvref_t<T>::value_type> &&
            std::derived_from<typename std::remove_cvref_t<T>::value_type::element_type, AstNode>) ||
        IsInnerAstNode<typename std::remove_cvref_t<T>::value_type>
    );
}
