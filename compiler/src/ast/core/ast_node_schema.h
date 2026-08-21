#pragma once

#include <tuple>
#include <type_traits>
#include <concepts>
#include <utility>
#include <cstddef>
#include <string>
#include <optional>
#include <vector>
#include <memory>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename Tuple>
    struct MemberSizeSum;

    template <typename... Members>
    struct MemberSizeSum<std::tuple<Members...>>
    {
        template <typename T>
        static constexpr size_t calculate()
        {
            return (sizeof(std::declval<T>().*std::declval<Members>()) + ... + 0);
        }
    };

    template <typename T>
    struct AstNodeSchema;

    template <typename T>
    concept HasAstNodeSchema = requires
    {
        { AstNodeSchema<T>::members };
    };

    template <typename T, size_t ExpectedDifference>
    concept SchemaMatchesLayout =
        HasAstNodeSchema<T> &&
        ((sizeof(T) - MemberSizeSum<std::remove_cvref_t<decltype(AstNodeSchema<T>::members)>>::template calculate<T>()) == ExpectedDifference);

    template <typename T, typename Func>
    requires HasAstNodeSchema<T>
    inline void for_each_ast_member(const T& node, Func&& func)
    {
        std::apply([&](auto... member_ptr) {
            (func(node.*member_ptr), ...);
        }, AstNodeSchema<T>::members);
    }

    template <typename T, typename Func>
    requires HasAstNodeSchema<T>
    inline void for_each_ast_member_pair(const T& a, const T& b, Func&& func)
    {
        std::apply([&](auto... member_ptr) {
            (func(a.*member_ptr, b.*member_ptr), ...);
        }, AstNodeSchema<T>::members);
    }

    template <>
    struct AstNodeSchema<Comment>
    {
        static constexpr auto members = std::make_tuple(
            &Comment::span,
            &Comment::text
        );
        static_assert(SchemaMatchesLayout<Comment, 16>);
    };

    template <>
    struct AstNodeSchema<CallArgument>
    {
        static constexpr auto members = std::make_tuple(
            &CallArgument::span,
            &CallArgument::name,
            &CallArgument::value
        );
        static_assert(SchemaMatchesLayout<CallArgument, 16>);
    };

    template <>
    struct AstNodeSchema<Modifier>
    {
        static constexpr auto members = std::make_tuple(
            &Modifier::span,
            &Modifier::name,
            &Modifier::arguments
        );
        static_assert(SchemaMatchesLayout<Modifier, 16>);
    };

    template <>
    struct AstNodeSchema<FunctionParameter>
    {
        static constexpr auto members = std::make_tuple(
            &FunctionParameter::span,
            &FunctionParameter::modifiers,
            &FunctionParameter::name,
            &FunctionParameter::type,
            &FunctionParameter::default_value
        );
        static_assert(SchemaMatchesLayout<FunctionParameter, 16>);
    };

    template <>
    struct AstNodeSchema<StructField>
    {
        static constexpr auto members = std::make_tuple(
            &StructField::span,
            &StructField::modifiers,
            &StructField::name,
            &StructField::type
        );
        static_assert(SchemaMatchesLayout<StructField, 16>);
    };

    template <>
    struct AstNodeSchema<EnumCase>
    {
        static constexpr auto members = std::make_tuple(
            &EnumCase::span,
            &EnumCase::modifiers,
            &EnumCase::name,
            &EnumCase::value
        );
        static_assert(SchemaMatchesLayout<EnumCase, 16>);
    };

    template <>
    struct AstNodeSchema<DictItem>
    {
        static constexpr auto members = std::make_tuple(
            &DictItem::span,
            &DictItem::modifiers,
            &DictItem::key,
            &DictItem::value
        );
        static_assert(SchemaMatchesLayout<DictItem, 16>);
    };

    template <>
    struct AstNodeSchema<SwitchCase>
    {
        static constexpr auto members = std::make_tuple(
            &SwitchCase::span,
            &SwitchCase::modifiers,
            &SwitchCase::identifiers,
            &SwitchCase::result
        );
        static_assert(SchemaMatchesLayout<SwitchCase, 16>);
    };

    template <>
    struct AstNodeSchema<AssignmentTarget>
    {
        static constexpr auto members = std::make_tuple(
            &AssignmentTarget::span,
            &AssignmentTarget::modifiers,
            &AssignmentTarget::name,
            &AssignmentTarget::type
        );
        static_assert(SchemaMatchesLayout<AssignmentTarget, 16>);
    };

    template <>
    struct AstNodeSchema<NumberLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &NumberLiteral::span,
            &NumberLiteral::value
        );
        static_assert(SchemaMatchesLayout<NumberLiteral, 16>);
    };

    template <>
    struct AstNodeSchema<PercentageLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &PercentageLiteral::span,
            &PercentageLiteral::value
        );
        static_assert(SchemaMatchesLayout<PercentageLiteral, 16>);
    };

    template <>
    struct AstNodeSchema<StringLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &StringLiteral::span,
            &StringLiteral::value
        );
        static_assert(SchemaMatchesLayout<StringLiteral, 16>);
    };

    template <>
    struct AstNodeSchema<BooleanLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &BooleanLiteral::span,
            &BooleanLiteral::value
        );
        static_assert(SchemaMatchesLayout<BooleanLiteral, 15>);
    };

    template <>
    struct AstNodeSchema<IdentifierAccess>
    {
        static constexpr auto members = std::make_tuple(
            &IdentifierAccess::span,
            &IdentifierAccess::name
        );
        static_assert(SchemaMatchesLayout<IdentifierAccess, 16>);
    };

    template <>
    struct AstNodeSchema<SelfExpression>
    {
        static constexpr auto members = std::make_tuple(
            &SelfExpression::span
        );
        static_assert(SchemaMatchesLayout<SelfExpression, 16>);
    };

    template <>
    struct AstNodeSchema<BinaryExpression>
    {
        static constexpr auto members = std::make_tuple(
            &BinaryExpression::span,
            &BinaryExpression::left,
            &BinaryExpression::op,
            &BinaryExpression::right
        );
        static_assert(SchemaMatchesLayout<BinaryExpression, 20>);
    };

    template <>
    struct AstNodeSchema<UnaryExpression>
    {
        static constexpr auto members = std::make_tuple(
            &UnaryExpression::span,
            &UnaryExpression::op,
            &UnaryExpression::right
        );
        static_assert(SchemaMatchesLayout<UnaryExpression, 12>);
    };

    template <>
    struct AstNodeSchema<GroupingExpression>
    {
        static constexpr auto members = std::make_tuple(
            &GroupingExpression::span,
            &GroupingExpression::expression
        );
        static_assert(SchemaMatchesLayout<GroupingExpression, 16>);
    };

    template <>
    struct AstNodeSchema<ConditionalExpression>
    {
        static constexpr auto members = std::make_tuple(
            &ConditionalExpression::span,
            &ConditionalExpression::condition,
            &ConditionalExpression::then_branch,
            &ConditionalExpression::else_branch
        );
        static_assert(SchemaMatchesLayout<ConditionalExpression, 16>);
    };

    template <>
    struct AstNodeSchema<FunctionCall>
    {
        static constexpr auto members = std::make_tuple(
            &FunctionCall::span,
            &FunctionCall::target,
            &FunctionCall::arguments
        );
        static_assert(SchemaMatchesLayout<FunctionCall, 16>);
    };

    template <>
    struct AstNodeSchema<DictLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &DictLiteral::span,
            &DictLiteral::elements
        );
        static_assert(SchemaMatchesLayout<DictLiteral, 16>);
    };

    template <>
    struct AstNodeSchema<TensorLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &TensorLiteral::span,
            &TensorLiteral::elements
        );
        static_assert(SchemaMatchesLayout<TensorLiteral, 16>);
    };

    template <>
    struct AstNodeSchema<TupleLiteral>
    {
        static constexpr auto members = std::make_tuple(
            &TupleLiteral::span,
            &TupleLiteral::elements
        );
        static_assert(SchemaMatchesLayout<TupleLiteral, 16>);
    };

    template <>
    struct AstNodeSchema<BracketAccess>
    {
        static constexpr auto members = std::make_tuple(
            &BracketAccess::span,
            &BracketAccess::target,
            &BracketAccess::index
        );
        static_assert(SchemaMatchesLayout<BracketAccess, 16>);
    };

    template <>
    struct AstNodeSchema<DotAccess>
    {
        static constexpr auto members = std::make_tuple(
            &DotAccess::span,
            &DotAccess::target,
            &DotAccess::property_name
        );
        static_assert(SchemaMatchesLayout<DotAccess, 16>);
    };

    template <>
    struct AstNodeSchema<SwitchExpression>
    {
        static constexpr auto members = std::make_tuple(
            &SwitchExpression::span,
            &SwitchExpression::target,
            &SwitchExpression::cases,
            &SwitchExpression::default_modifiers,
            &SwitchExpression::default_case
        );
        static_assert(SchemaMatchesLayout<SwitchExpression, 16>);
    };

    template <>
    struct AstNodeSchema<Assignment>
    {
        static constexpr auto members = std::make_tuple(
            &Assignment::span,
            &Assignment::targets,
            &Assignment::value
        );
        static_assert(SchemaMatchesLayout<Assignment, 16>);
    };

    template <>
    struct AstNodeSchema<Reassignment>
    {
        static constexpr auto members = std::make_tuple(
            &Reassignment::span,
            &Reassignment::target,
            &Reassignment::value
        );
        static_assert(SchemaMatchesLayout<Reassignment, 16>);
    };

    template <>
    struct AstNodeSchema<ExpressionStatement>
    {
        static constexpr auto members = std::make_tuple(
            &ExpressionStatement::span,
            &ExpressionStatement::expr
        );
        static_assert(SchemaMatchesLayout<ExpressionStatement, 16>);
    };

    template <>
    struct AstNodeSchema<ReturnStatement>
    {
        static constexpr auto members = std::make_tuple(
            &ReturnStatement::span,
            &ReturnStatement::modifiers,
            &ReturnStatement::values
        );
        static_assert(SchemaMatchesLayout<ReturnStatement, 16>);
    };

    template <>
    struct AstNodeSchema<EnumDefinition>
    {
        static constexpr auto members = std::make_tuple(
            &EnumDefinition::span,
            &EnumDefinition::modifiers,
            &EnumDefinition::name,
            &EnumDefinition::underlying_type,
            &EnumDefinition::cases
        );
        static_assert(SchemaMatchesLayout<EnumDefinition, 16>);
    };

    template <>
    struct AstNodeSchema<Directive>
    {
        static constexpr auto members = std::make_tuple(
            &Directive::span,
            &Directive::name,
            &Directive::value
        );
        static_assert(SchemaMatchesLayout<Directive, 16>);
    };

    template <>
    struct AstNodeSchema<ImportStatement>
    {
        static constexpr auto members = std::make_tuple(
            &ImportStatement::span,
            &ImportStatement::modifiers,
            &ImportStatement::path,
            &ImportStatement::resolved_canonical_path
        );
        static_assert(SchemaMatchesLayout<ImportStatement, 16>);
    };

    template <>
    struct AstNodeSchema<FunctionDefinition>
    {
        static constexpr auto members = std::make_tuple(
            &FunctionDefinition::span,
            &FunctionDefinition::modifiers,
            &FunctionDefinition::name,
            &FunctionDefinition::parameters,
            &FunctionDefinition::return_types,
            &FunctionDefinition::body,
            &FunctionDefinition::docstring
        );
        static_assert(SchemaMatchesLayout<FunctionDefinition, 16>);
    };

    template <>
    struct AstNodeSchema<StructDefinition>
    {
        static constexpr auto members = std::make_tuple(
            &StructDefinition::span,
            &StructDefinition::modifiers,
            &StructDefinition::name,
            &StructDefinition::fields
        );
        static_assert(SchemaMatchesLayout<StructDefinition, 16>);
    };

    template <>
    struct AstNodeSchema<TypeAliasDefinition>
    {
        static constexpr auto members = std::make_tuple(
            &TypeAliasDefinition::span,
            &TypeAliasDefinition::modifiers,
            &TypeAliasDefinition::name,
            &TypeAliasDefinition::target_type
        );
        static_assert(SchemaMatchesLayout<TypeAliasDefinition, 16>);
    };

    template <>
    struct AstNodeSchema<ExtensionDefinition>
    {
        static constexpr auto members = std::make_tuple(
            &ExtensionDefinition::span,
            &ExtensionDefinition::modifiers,
            &ExtensionDefinition::target_type,
            &ExtensionDefinition::execution_steps,
            &ExtensionDefinition::function_definitions,
            &ExtensionDefinition::struct_definitions,
            &ExtensionDefinition::enum_definitions,
            &ExtensionDefinition::type_aliases
        );
        static_assert(SchemaMatchesLayout<ExtensionDefinition, 16>);
    };

    template <>
    struct AstNodeSchema<Program>
    {
        static constexpr auto members = std::make_tuple(
            &Program::span,
            &Program::comments,
            &Program::import_statements,
            &Program::directives,
            &Program::execution_steps,
            &Program::function_definitions,
            &Program::struct_definitions,
            &Program::enum_definitions,
            &Program::type_aliases,
            &Program::extension_definitions
        );
        static_assert(SchemaMatchesLayout<Program, 24>);
    };

    template <>
    struct AstNodeSchema<TypeAnnotation>
    {
        static constexpr auto members = std::make_tuple(
            &TypeAnnotation::span,
            &TypeAnnotation::name,
            &TypeAnnotation::generic_args
        );
        static_assert(SchemaMatchesLayout<TypeAnnotation, 16>);
    };

    template <>
    struct AstNodeSchema<TupleTypeAnnotation>
    {
        static constexpr auto members = std::make_tuple(
            &TupleTypeAnnotation::span,
            &TupleTypeAnnotation::name,
            &TupleTypeAnnotation::generic_args,
            &TupleTypeAnnotation::element_types
        );
        static_assert(SchemaMatchesLayout<TupleTypeAnnotation, 16>);
    };

    template <HasAstNodeSchema T>
    consteval bool verify_has_ast_node_schema()
    {
        return true;
    }

    template <typename Tuple>
    struct AstSchemaCoverageValidator;

    template <typename... Types>
    struct AstSchemaCoverageValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_has_ast_node_schema<Types>() && ...);
        }
    };

    static_assert(AstSchemaCoverageValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node type in AllAstNodeTypes must have an implemented AstNodeSchema specialization");
}
