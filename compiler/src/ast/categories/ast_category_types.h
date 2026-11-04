#pragma once

#include <tuple>
#include <string_view>
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_type.h"
#include "ast/categories/ast_statement_types.h"
#include "ast/categories/ast_expression_types.h"
#include "ast/categories/ast_type_annotation_types.h"

namespace valuascript::compiler
{
    using AstCategoryTypes = std::tuple<
        Statement,
        Expression,
        TypeAnnotation
    >;

    template <typename Category>
    struct CategoryNodeTypes;

    template <>
    struct CategoryNodeTypes<Statement>
    {
        using type = AllStatementNodeTypes;
        static constexpr std::string_view name = "Statement";
    };

    template <>
    struct CategoryNodeTypes<Expression>
    {
        using type = AllExpressionNodeTypes;
        static constexpr std::string_view name = "Expression";
    };

    template <>
    struct CategoryNodeTypes<TypeAnnotation>
    {
        using type = AllTypeAnnotationNodeTypes;
        static constexpr std::string_view name = "TypeAnnotation";
    };

    template <typename Category>
    using category_node_types_t = typename CategoryNodeTypes<Category>::type;

    template <typename Category>
    inline constexpr std::string_view category_name_v = CategoryNodeTypes<Category>::name;
}
