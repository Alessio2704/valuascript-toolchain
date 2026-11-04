#pragma once

#include <tuple>
#include "utils/traits/tuple_traits.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_concepts.h"

#include "ast/categories/ast_declaration_types.h"
#include "ast/categories/ast_statement_types.h"
#include "ast/categories/ast_expression_types.h"
#include "ast/categories/ast_type_annotation_types.h"
#include "ast/categories/ast_inner_types.h"
#include "ast/categories/ast_category_types.h"

namespace valuascript::compiler
{
    using AllAstNodeTypes = valuascript::shared::tuple_concat_t<
        std::tuple<Program>,
        AllDeclarationNodeTypes,
        AllStatementNodeTypes,
        AllExpressionNodeTypes,
        AllTypeAnnotationNodeTypes,
        AllInnerNodeTypes
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
                  "AllAstNodeTypes contains duplicate C++ types across category partitions!");

    static_assert(valuascript::shared::UniqueTupleTags<AllAstNodeTypes>,
                  "AllAstNodeTypes contains classes with duplicate AstKind values!");

    static_assert(AstNodeCompletenessValidator<AllAstNodeTypes>::validate(),
                  "All registered AST node types must derive from AstNode and define static constexpr AstKind KIND");

    static_assert(std::tuple_size_v<AllAstNodeTypes> == AST_KIND_COUNT,
                  "AllAstNodeTypes registry count mismatch: The total number of categorized AST classes does not match AST_KIND_COUNT!");

    static_assert(valuascript::shared::tuple_all_derive_from_v<Statement, AllStatementNodeTypes>,
                  "All types in AllStatementNodeTypes must derive from Statement!");

    static_assert(valuascript::shared::tuple_all_derive_from_v<Expression, AllExpressionNodeTypes>,
                  "All types in AllExpressionNodeTypes must derive from Expression!");

    static_assert(valuascript::shared::tuple_all_derive_from_v<TypeAnnotation, AllTypeAnnotationNodeTypes>,
                  "All types in AllTypeAnnotationNodeTypes must derive from TypeAnnotation!");

    static_assert(valuascript::shared::tuple_all_derive_from_v<AstNode, AllDeclarationNodeTypes>,
                  "All types in AllDeclarationNodeTypes must derive from AstNode!");

    static_assert(valuascript::shared::tuple_all_derive_from_v<AstNode, AllInnerNodeTypes>,
                  "All types in AllInnerNodeTypes must derive from AstNode!");

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

    template <typename Fn>
    constexpr void for_each_expression_type(Fn&& fn)
    {
        valuascript::shared::tuple_for_each_type<AllExpressionNodeTypes>(std::forward<Fn>(fn));
    }

    template <typename Fn>
    constexpr void for_each_statement_type(Fn&& fn)
    {
        valuascript::shared::tuple_for_each_type<AllStatementNodeTypes>(std::forward<Fn>(fn));
    }

    template <typename Fn>
    constexpr void for_each_declaration_type(Fn&& fn)
    {
        valuascript::shared::tuple_for_each_type<AllDeclarationNodeTypes>(std::forward<Fn>(fn));
    }

    template <typename Fn>
    constexpr void for_each_type_annotation_type(Fn&& fn)
    {
        valuascript::shared::tuple_for_each_type<AllTypeAnnotationNodeTypes>(std::forward<Fn>(fn));
    }

    template <typename Fn>
    constexpr void for_each_inner_node_type(Fn&& fn)
    {
        valuascript::shared::tuple_for_each_type<AllInnerNodeTypes>(std::forward<Fn>(fn));
    }

    template <typename Fn>
    constexpr void for_each_ast_node_type(Fn&& fn)
    {
        valuascript::shared::tuple_for_each_type<AllAstNodeTypes>(std::forward<Fn>(fn));
    }
}
