#pragma once

#include <tuple>
#include <type_traits>
#include <concepts>
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"

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

    static_assert(std::tuple_size_v<AllAstNodeTypes> == 40,
                  "AllAstNodeTypes registry count mismatch: Did you add a new AST node type without updating the master type registry?")
    ;

    template <typename Tuple>
    struct ValidateAstNodeTypes;

    template <typename... Types>
    struct ValidateAstNodeTypes<std::tuple<Types...>>
    {
        static constexpr bool value = (
            (std::derived_from<Types, AstNode> && requires { { Types::KIND } -> std::same_as<const AstKind&>; }) && ...
        );
    };

    static_assert(ValidateAstNodeTypes<AllAstNodeTypes>::value,
                  "All registered AST node types must derive from AstNode and define static constexpr AstKind KIND");

    using AstCategoryTypes = std::tuple<
        Statement,
        Expression,
        TypeAnnotation
    >;

    template <typename Tuple>
    struct ValidateAstCategoryTypes;

    template <typename... Categories>
    struct ValidateAstCategoryTypes<std::tuple<Categories...>>
    {
        static constexpr bool value = (std::derived_from<Categories, AstNode> && ...);
    };

    static_assert(ValidateAstCategoryTypes<AstCategoryTypes>::value,
                  "All registered category types in AstCategoryTypes must derive from AstNode");
}
