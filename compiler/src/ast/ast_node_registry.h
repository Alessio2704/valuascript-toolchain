#pragma once

#include <tuple>
#include <type_traits>
#include <concepts>
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "token/comment_token.h"

namespace valuascript::compiler
{
    // =========================================================================
    // Master AST Node Type Registry
    // Every concrete AstNode MUST be listed here.
    // =========================================================================
    using AllAstNodeTypes = std::tuple<
        // Top-Level
        Program,

        // Declarations
        ImportStatement,
        Directive,
        FunctionDefinition,
        StructDefinition,
        EnumDefinition,
        TypeAliasDefinition,
        ExtensionDefinition,

        // Statements
        Assignment,
        Reassignment,
        ExpressionStatement,
        ReturnStatement,

        // Expressions
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

        // Types
        TypeAnnotation,
        TupleTypeAnnotation
    >;

    // Total Count Verification (31 concrete AST node types)
    static_assert(std::tuple_size_v<AllAstNodeTypes> == 31, 
        "AllAstNodeTypes registry count mismatch: Did you add a new AST node type without updating the master type registry?");

    // =========================================================================
    // Master Non-Node Grammar Structs Registry
    // Every non-node grammar struct / trivia type MUST be listed here.
    // =========================================================================
    using AllGrammarStructs = std::tuple<
        Modifier,
        CallArgument,
        FunctionParameter,
        StructField,
        EnumCase,
        DictItem,
        SwitchCase,
        AssignmentTarget,
        CommentToken
    >;

    static_assert(std::tuple_size_v<AllGrammarStructs> == 9,
        "AllGrammarStructs count mismatch: Did you add a new grammar struct without updating the master grammar registry?");

    // =========================================================================
    // Master AST Category Hierarchy Types
    // =========================================================================
    using AllCategoryTypes = std::tuple<
        AstNode,
        Statement,
        Expression,
        TypeAnnotation
    >;

    static_assert(std::tuple_size_v<AllCategoryTypes> == 4,
        "AllCategoryTypes count mismatch: Did you add a new AST category type without updating the category registry?");

    // Verify every registered type satisfies AstNodeSubclass and has KIND
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
}
