#pragma once

#include <tuple>
#include <type_traits>
#include <concepts>
#include "utils/traits/tuple_traits.h"
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "ast_concepts.h"

namespace valuascript::compiler
{
    using AllAstNodeTypes = std::tuple<
        Program,
        Comment,
        ImportStatement,
        Directive,
        FunctionDefinition,
        StructDefinition,
        EnumDefinition,
        TypeAliasDefinition,
        ExtensionDefinition,
        Assignment,
        Reassignment,
        ExpressionStatement,
        ReturnStatement,
        NumberLiteral,
        PercentageLiteral,
        StringLiteral,
        BooleanLiteral,
        IdentifierAccess,
        SelfExpression,
        BinaryExpression,
        UnaryExpression,
        GroupingExpression,
        ConditionalExpression,
        FunctionCall,
        DictLiteral,
        TensorLiteral,
        TupleLiteral,
        BracketAccess,
        DotAccess,
        SwitchExpression,
        TypeAnnotation,
        TupleTypeAnnotation,
        FunctionParameter,
        StructField,
        EnumCase,
        SwitchCase,
        AssignmentTarget,
        Modifier,
        CallArgument,
        DictItem
    >;

    template <typename Tuple>
    struct NodeDispatcher;

    template <typename... Types>
    struct NodeDispatcher<std::tuple<Types...>>
    {
        template <typename Fn>
        static bool dispatch(AstKind kind, Fn&& fn)
        {
            return ((kind == Types::KIND ? (fn.template operator()<Types>(), true) : false) || ...);
        }
    };

    template <AstKind K, typename Tuple>
    struct TupleContainsKind;

    template <AstKind K>
    struct TupleContainsKind<K, std::tuple<>> : std::false_type {};

    template <AstKind K, typename Head, typename... Tail>
    struct TupleContainsKind<K, std::tuple<Head, Tail...>>
        : std::conditional_t<Head::KIND == K, std::true_type, TupleContainsKind<K, std::tuple<Tail...>>> {};

    template <typename Tuple>
    struct AreNodeKindsUnique;

    template <>
    struct AreNodeKindsUnique<std::tuple<>> : std::true_type {};

    template <typename Head, typename... Tail>
    struct AreNodeKindsUnique<std::tuple<Head, Tail...>>
        : std::conditional_t<
            TupleContainsKind<Head::KIND, std::tuple<Tail...>>::value,
            std::false_type,
            AreNodeKindsUnique<std::tuple<Tail...>>
        > {};

    template <typename Tuple>
    concept UniqueNodeKinds = AreNodeKindsUnique<Tuple>::value;

    template <ConcreteAstNode T>
    consteval bool verify_concrete_ast_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstNodeCompletenessValidator;

    template <typename... Types>
    struct AstNodeCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_concrete_ast_node<Types>() && ...);
        }
    };

    static_assert(valuascript::shared::UniqueTuple<AllAstNodeTypes>,
                  "AllAstNodeTypes contains duplicate C++ types!");

    static_assert(UniqueNodeKinds<AllAstNodeTypes>,
                  "AllAstNodeTypes contains classes with duplicate AstKind values!");

    static_assert(AstNodeCompletenessValidator<AllAstNodeTypes>::validate(),
                  "All registered AST node types must derive from AstNode and define static constexpr AstKind KIND");

    static_assert(std::tuple_size_v<AllAstNodeTypes> == AST_KIND_COUNT,
                  "AllAstNodeTypes registry count mismatch: The number of registered AST classes does not match the non-Unknown cases in AstKind!");

    using AstCategoryTypes = std::tuple<
        Statement,
        Expression,
        TypeAnnotation
    >;

    template <AstElement Category>
    consteval bool verify_ast_element()
    {
        return true;
    }

    template <typename Tuple>
    struct AstCategoryCompletenessValidator;

    template <typename... Categories>
    struct AstCategoryCompletenessValidator<std::tuple<Categories...>>
    {
        static consteval bool validate()
        {
            return (verify_ast_element<Categories>() && ...);
        }
    };

    static_assert(valuascript::shared::UniqueTuple<AstCategoryTypes>,
                  "AstCategoryTypes contains duplicate category types!");

    static_assert(AstCategoryCompletenessValidator<AstCategoryTypes>::validate(),
                  "All registered category types in AstCategoryTypes must derive from AstNode");
}
