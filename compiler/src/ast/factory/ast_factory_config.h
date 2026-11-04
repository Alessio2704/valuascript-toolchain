#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <optional>
#include <tuple>
#include <type_traits>
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/categories/ast_expression_types.h"
#include "ast/categories/ast_statement_types.h"
#include "ast/categories/ast_type_annotation_types.h"
#include "ast/categories/ast_declaration_types.h"
#include "ast/categories/ast_reassignment_target_types.h"

namespace valuascript::compiler
{
    template <auto MemberPtr>
    struct VectorFieldConfig;

    template <typename ClassT, typename ElemT, std::vector<ElemT> ClassT::*MemberPtr>
    struct VectorFieldConfig<MemberPtr>
    {
        static constexpr auto member_ptr = MemberPtr;
        using element_type = ElemT;
        size_t count = 3;
    };

    template <auto MemberPtr>
    struct NameFieldConfig;

    template <typename ClassT, NodeName ClassT::*MemberPtr>
    struct NameFieldConfig<MemberPtr>
    {
        static constexpr auto member_ptr = MemberPtr;
        std::string prefix = "name";
    };

    template <auto MemberPtr>
    struct OperatorFieldConfig;

    template <typename ClassT, TokenType ClassT::*MemberPtr>
    struct OperatorFieldConfig<MemberPtr>
    {
        static constexpr auto member_ptr = MemberPtr;
        TokenType op = TokenType::Plus;
    };

    template <auto MemberPtr>
    struct StringFieldConfig;

    template <typename ClassT, std::string ClassT::*MemberPtr>
    struct StringFieldConfig<MemberPtr>
    {
        static constexpr auto member_ptr = MemberPtr;
        std::string value = "";
    };

    template <auto MemberPtr>
    struct BoolFieldConfig;

    template <typename ClassT, bool ClassT::*MemberPtr>
    struct BoolFieldConfig<MemberPtr>
    {
        static constexpr auto member_ptr = MemberPtr;
        bool value = true;
    };

    template <auto MemberPtr, typename ValueT = bool>
    struct OptionalFieldConfig;

    template <typename ClassT, typename MemberT, MemberT ClassT::*MemberPtr, typename ValueT>
    struct OptionalFieldConfig<MemberPtr, ValueT>
    {
        static constexpr auto member_ptr = MemberPtr;
        using value_type = ValueT;
        std::optional<ValueT> value = std::nullopt;
    };

    template <typename T>
    struct NodeConfig;

    template <>
    struct NodeConfig<Comment>
    {
        StringFieldConfig<&Comment::text> text{.value = "comment"};
        static constexpr auto members = std::make_tuple(&NodeConfig::text);
    };

    template <>
    struct NodeConfig<CallArgument>
    {
        NameFieldConfig<&CallArgument::name> name{.prefix = "arg"};
        static constexpr auto members = std::make_tuple(&NodeConfig::name);
    };

    template <>
    struct NodeConfig<Modifier>
    {
        NameFieldConfig<&Modifier::name> name{.prefix = "mod"};
        VectorFieldConfig<&Modifier::arguments> arguments{.count = 2};
        static constexpr auto members = std::make_tuple(&NodeConfig::name, &NodeConfig::arguments);
    };

    template <>
    struct NodeConfig<FunctionParameter>
    {
        VectorFieldConfig<&FunctionParameter::modifiers> modifiers{.count = 2};
        NameFieldConfig<&FunctionParameter::name> name{.prefix = "param"};
        OptionalFieldConfig<&FunctionParameter::default_value, bool> default_value{.value = true};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name,
                                                        &NodeConfig::default_value);
    };

    template <>
    struct NodeConfig<StructField>
    {
        VectorFieldConfig<&StructField::modifiers> modifiers{.count = 2};
        NameFieldConfig<&StructField::name> name{.prefix = "field"};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name);
    };

    template <>
    struct NodeConfig<EnumCase>
    {
        VectorFieldConfig<&EnumCase::modifiers> modifiers{.count = 2};
        NameFieldConfig<&EnumCase::name> name{.prefix = "Case"};
        OptionalFieldConfig<&EnumCase::value, bool> value{.value = true};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name, &NodeConfig::value);
    };

    template <>
    struct NodeConfig<DictItem>
    {
        VectorFieldConfig<&DictItem::modifiers> modifiers{.count = 2};
        NameFieldConfig<&DictItem::key> key{.prefix = "key"};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::key);
    };

    template <>
    struct NodeConfig<SwitchCase>
    {
        VectorFieldConfig<&SwitchCase::modifiers> modifiers{.count = 2};
        VectorFieldConfig<&SwitchCase::identifiers> identifiers{.count = 2};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::identifiers);
    };

    template <>
    struct NodeConfig<AssignmentTarget>
    {
        VectorFieldConfig<&AssignmentTarget::modifiers> modifiers{.count = 2};
        NameFieldConfig<&AssignmentTarget::name> name{.prefix = "target"};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name);
    };

    template <>
    struct NodeConfig<NumberLiteral>
    {
        StringFieldConfig<&NumberLiteral::value> value{.value = "42"};
        static constexpr auto members = std::make_tuple(&NodeConfig::value);
    };

    template <>
    struct NodeConfig<PercentageLiteral>
    {
        StringFieldConfig<&PercentageLiteral::value> value{.value = "50%"};
        static constexpr auto members = std::make_tuple(&NodeConfig::value);
    };

    template <>
    struct NodeConfig<StringLiteral>
    {
        StringFieldConfig<&StringLiteral::value> value{.value = "sample_string"};
        static constexpr auto members = std::make_tuple(&NodeConfig::value);
    };

    template <>
    struct NodeConfig<BooleanLiteral>
    {
        BoolFieldConfig<&BooleanLiteral::value> value{.value = true};
        static constexpr auto members = std::make_tuple(&NodeConfig::value);
    };

    template <>
    struct NodeConfig<IdentifierAccess>
    {
        NameFieldConfig<&IdentifierAccess::name> name{.prefix = "identifier"};
        static constexpr auto members = std::make_tuple(&NodeConfig::name);
    };

    template <>
    struct NodeConfig<SelfExpression>
    {
        static constexpr auto members = std::make_tuple();
    };

    template <>
    struct NodeConfig<BinaryExpression>
    {
        OperatorFieldConfig<&BinaryExpression::op> op{.op = TokenType::Plus};
        static constexpr auto members = std::make_tuple(&NodeConfig::op);
    };

    template <>
    struct NodeConfig<UnaryExpression>
    {
        OperatorFieldConfig<&UnaryExpression::op> op{.op = TokenType::Minus};
        static constexpr auto members = std::make_tuple(&NodeConfig::op);
    };

    template <>
    struct NodeConfig<GroupingExpression>
    {
        static constexpr auto members = std::make_tuple();
    };

    template <>
    struct NodeConfig<ConditionalExpression>
    {
        static constexpr auto members = std::make_tuple();
    };

    template <>
    struct NodeConfig<FunctionCall>
    {
        VectorFieldConfig<&FunctionCall::arguments> arguments{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::arguments);
    };

    template <>
    struct NodeConfig<DictLiteral>
    {
        VectorFieldConfig<&DictLiteral::elements> elements{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::elements);
    };

    template <>
    struct NodeConfig<TensorLiteral>
    {
        VectorFieldConfig<&TensorLiteral::elements> elements{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::elements);
    };

    template <>
    struct NodeConfig<TupleLiteral>
    {
        VectorFieldConfig<&TupleLiteral::elements> elements{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::elements);
    };

    template <>
    struct NodeConfig<BracketAccess>
    {
        static constexpr auto members = std::make_tuple();
    };

    template <>
    struct NodeConfig<DotAccess>
    {
        NameFieldConfig<&DotAccess::property_name> property_name{.prefix = "prop"};
        static constexpr auto members = std::make_tuple(&NodeConfig::property_name);
    };

    template <>
    struct NodeConfig<SwitchExpression>
    {
        VectorFieldConfig<&SwitchExpression::cases> cases{.count = 3};
        VectorFieldConfig<&SwitchExpression::default_modifiers> default_modifiers{.count = 2};
        OptionalFieldConfig<&SwitchExpression::default_case, bool> default_case{.value = true};
        static constexpr auto members = std::make_tuple(&NodeConfig::cases, &NodeConfig::default_modifiers,
                                                        &NodeConfig::default_case);
    };

    template <>
    struct NodeConfig<Assignment>
    {
        VectorFieldConfig<&Assignment::targets> targets{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::targets);
    };

    template <>
    struct NodeConfig<Reassignment>
    {
        static constexpr auto members = std::make_tuple();
    };

    template <>
    struct NodeConfig<ExpressionStatement>
    {
        static constexpr auto members = std::make_tuple();
    };

    template <>
    struct NodeConfig<ReturnStatement>
    {
        VectorFieldConfig<&ReturnStatement::modifiers> modifiers{.count = 2};
        VectorFieldConfig<&ReturnStatement::values> values{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::values);
    };

    template <>
    struct NodeConfig<EnumDefinition>
    {
        VectorFieldConfig<&EnumDefinition::modifiers> modifiers{.count = 2};
        NameFieldConfig<&EnumDefinition::name> name{.prefix = "EnumDef"};
        VectorFieldConfig<&EnumDefinition::cases> cases{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name, &NodeConfig::cases);
    };

    template <>
    struct NodeConfig<Directive>
    {
        NameFieldConfig<&Directive::name> name{.prefix = "directive_name"};
        static constexpr auto members = std::make_tuple(&NodeConfig::name);
    };

    template <>
    struct NodeConfig<ImportStatement>
    {
        VectorFieldConfig<&ImportStatement::modifiers> modifiers{.count = 2};
        NameFieldConfig<&ImportStatement::path> path{.prefix = "import.module.path"};
        OptionalFieldConfig<&ImportStatement::resolved_canonical_path, std::string> resolved_canonical_path{
            .value = std::nullopt
        };
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::path,
                                                        &NodeConfig::resolved_canonical_path);
    };

    template <>
    struct NodeConfig<FunctionDefinition>
    {
        VectorFieldConfig<&FunctionDefinition::modifiers> modifiers{.count = 2};
        NameFieldConfig<&FunctionDefinition::name> name{.prefix = "func_name"};
        VectorFieldConfig<&FunctionDefinition::parameters> parameters{.count = 3};
        VectorFieldConfig<&FunctionDefinition::return_types> return_types{.count = 2};
        VectorFieldConfig<&FunctionDefinition::body> body{.count = 3};
        OptionalFieldConfig<&FunctionDefinition::docstring, std::string> docstring{
            .value = "Docstring for sample function"
        };
        static constexpr auto members = std::make_tuple(
            &NodeConfig::modifiers,
            &NodeConfig::name,
            &NodeConfig::parameters,
            &NodeConfig::return_types,
            &NodeConfig::body,
            &NodeConfig::docstring
        );
    };

    template <>
    struct NodeConfig<StructDefinition>
    {
        VectorFieldConfig<&StructDefinition::modifiers> modifiers{.count = 2};
        NameFieldConfig<&StructDefinition::name> name{.prefix = "StructName"};
        VectorFieldConfig<&StructDefinition::fields> fields{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name, &NodeConfig::fields);
    };

    template <>
    struct NodeConfig<TypeAliasDefinition>
    {
        VectorFieldConfig<&TypeAliasDefinition::modifiers> modifiers{.count = 2};
        NameFieldConfig<&TypeAliasDefinition::name> name{.prefix = "AliasName"};
        static constexpr auto members = std::make_tuple(&NodeConfig::modifiers, &NodeConfig::name);
    };

    template <>
    struct NodeConfig<ExtensionDefinition>
    {
        VectorFieldConfig<&ExtensionDefinition::modifiers> modifiers{.count = 2};
        VectorFieldConfig<&ExtensionDefinition::execution_steps> execution_steps{.count = 2};
        VectorFieldConfig<&ExtensionDefinition::function_definitions> function_definitions{.count = 1};
        VectorFieldConfig<&ExtensionDefinition::struct_definitions> struct_definitions{.count = 1};
        VectorFieldConfig<&ExtensionDefinition::enum_definitions> enum_definitions{.count = 1};
        VectorFieldConfig<&ExtensionDefinition::type_aliases> type_aliases{.count = 1};
        static constexpr auto members = std::make_tuple(
            &NodeConfig::modifiers,
            &NodeConfig::execution_steps,
            &NodeConfig::function_definitions,
            &NodeConfig::struct_definitions,
            &NodeConfig::enum_definitions,
            &NodeConfig::type_aliases
        );
    };

    template <>
    struct NodeConfig<Program>
    {
        VectorFieldConfig<&Program::comments> comments{.count = 2};
        VectorFieldConfig<&Program::import_statements> import_statements{.count = 1};
        VectorFieldConfig<&Program::directives> directives{.count = 1};
        VectorFieldConfig<&Program::execution_steps> execution_steps{.count = 2};
        VectorFieldConfig<&Program::function_definitions> function_definitions{.count = 1};
        VectorFieldConfig<&Program::struct_definitions> struct_definitions{.count = 1};
        VectorFieldConfig<&Program::enum_definitions> enum_definitions{.count = 1};
        VectorFieldConfig<&Program::type_aliases> type_aliases{.count = 1};
        VectorFieldConfig<&Program::extension_definitions> extension_definitions{.count = 1};
        static constexpr auto members = std::make_tuple(
            &NodeConfig::comments,
            &NodeConfig::import_statements,
            &NodeConfig::directives,
            &NodeConfig::execution_steps,
            &NodeConfig::function_definitions,
            &NodeConfig::struct_definitions,
            &NodeConfig::enum_definitions,
            &NodeConfig::type_aliases,
            &NodeConfig::extension_definitions
        );
    };

    template <>
    struct NodeConfig<TypeAnnotation>
    {
        NameFieldConfig<&TypeAnnotation::name> name{.prefix = "TypeAnn"};
        VectorFieldConfig<&TypeAnnotation::generic_args> generic_args{.count = 0};
        static constexpr auto members = std::make_tuple(&NodeConfig::name, &NodeConfig::generic_args);
    };

    template <>
    struct NodeConfig<TupleTypeAnnotation>
    {
        NameFieldConfig<&TypeAnnotation::name> name{.prefix = ""};
        VectorFieldConfig<&TypeAnnotation::generic_args> generic_args{.count = 0};
        VectorFieldConfig<&TupleTypeAnnotation::element_types> element_types{.count = 3};
        static constexpr auto members = std::make_tuple(&NodeConfig::name, &NodeConfig::generic_args,
                                                        &NodeConfig::element_types);
    };

    struct AstFactoryGeneralConfig
    {
        int max_expression_depth = 4;
        int max_type_depth = 2;
        ExpressionKind expression_kind = ExpressionKind::from<NumberLiteral>();
        StatementKind statement_kind = StatementKind::from<ExpressionStatement>();
        TypeAnnotationKind type_kind = TypeAnnotationKind::from<TypeAnnotation>();
        ReassignmentTargetKind reassignment_target_kind = ReassignmentTargetKind::from<IdentifierAccess>();
    };

    struct AstFactoryConfig
    {
        AstFactoryGeneralConfig general{};

        NodeConfig<Comment> comment{};
        NodeConfig<CallArgument> call_argument{};
        NodeConfig<Modifier> modifier{};
        NodeConfig<FunctionParameter> function_parameter{};
        NodeConfig<StructField> struct_field{};
        NodeConfig<EnumCase> enum_case{};
        NodeConfig<DictItem> dict_item{};
        NodeConfig<SwitchCase> switch_case{};
        NodeConfig<AssignmentTarget> assignment_target{};

        NodeConfig<NumberLiteral> number_literal{};
        NodeConfig<PercentageLiteral> percentage_literal{};
        NodeConfig<StringLiteral> string_literal{};
        NodeConfig<BooleanLiteral> boolean_literal{};
        NodeConfig<IdentifierAccess> identifier_access{};
        NodeConfig<SelfExpression> self_expression{};

        NodeConfig<BinaryExpression> binary_expression{};
        NodeConfig<UnaryExpression> unary_expression{};
        NodeConfig<GroupingExpression> grouping_expression{};
        NodeConfig<ConditionalExpression> conditional_expression{};
        NodeConfig<FunctionCall> function_call{};
        NodeConfig<DictLiteral> dict_literal{};
        NodeConfig<TensorLiteral> tensor_literal{};
        NodeConfig<TupleLiteral> tuple_literal{};
        NodeConfig<BracketAccess> bracket_access{};
        NodeConfig<DotAccess> dot_access{};
        NodeConfig<SwitchExpression> switch_expression{};

        NodeConfig<Assignment> assignment{};
        NodeConfig<Reassignment> reassignment{};
        NodeConfig<ExpressionStatement> expression_statement{};
        NodeConfig<ReturnStatement> return_statement{};

        NodeConfig<EnumDefinition> enum_definition{};
        NodeConfig<Directive> directive{};
        NodeConfig<ImportStatement> import_statement{};
        NodeConfig<FunctionDefinition> function_definition{};
        NodeConfig<StructDefinition> struct_definition{};
        NodeConfig<TypeAliasDefinition> type_alias_definition{};
        NodeConfig<ExtensionDefinition> extension_definition{};
        NodeConfig<Program> program{};

        NodeConfig<TypeAnnotation> type_annotation{};
        NodeConfig<TupleTypeAnnotation> tuple_type_annotation{};

        template <typename T>
        constexpr auto& get()
        {
            if constexpr (std::is_same_v<T, Comment>) return comment;
            else if constexpr (std::is_same_v<T, CallArgument>) return call_argument;
            else if constexpr (std::is_same_v<T, Modifier>) return modifier;
            else if constexpr (std::is_same_v<T, FunctionParameter>) return function_parameter;
            else if constexpr (std::is_same_v<T, StructField>) return struct_field;
            else if constexpr (std::is_same_v<T, EnumCase>) return enum_case;
            else if constexpr (std::is_same_v<T, DictItem>) return dict_item;
            else if constexpr (std::is_same_v<T, SwitchCase>) return switch_case;
            else if constexpr (std::is_same_v<T, AssignmentTarget>) return assignment_target;
            else if constexpr (std::is_same_v<T, NumberLiteral>) return number_literal;
            else if constexpr (std::is_same_v<T, PercentageLiteral>) return percentage_literal;
            else if constexpr (std::is_same_v<T, StringLiteral>) return string_literal;
            else if constexpr (std::is_same_v<T, BooleanLiteral>) return boolean_literal;
            else if constexpr (std::is_same_v<T, IdentifierAccess>) return identifier_access;
            else if constexpr (std::is_same_v<T, SelfExpression>) return self_expression;
            else if constexpr (std::is_same_v<T, BinaryExpression>) return binary_expression;
            else if constexpr (std::is_same_v<T, UnaryExpression>) return unary_expression;
            else if constexpr (std::is_same_v<T, GroupingExpression>) return grouping_expression;
            else if constexpr (std::is_same_v<T, ConditionalExpression>) return conditional_expression;
            else if constexpr (std::is_same_v<T, FunctionCall>) return function_call;
            else if constexpr (std::is_same_v<T, DictLiteral>) return dict_literal;
            else if constexpr (std::is_same_v<T, TensorLiteral>) return tensor_literal;
            else if constexpr (std::is_same_v<T, TupleLiteral>) return tuple_literal;
            else if constexpr (std::is_same_v<T, BracketAccess>) return bracket_access;
            else if constexpr (std::is_same_v<T, DotAccess>) return dot_access;
            else if constexpr (std::is_same_v<T, SwitchExpression>) return switch_expression;
            else if constexpr (std::is_same_v<T, Assignment>) return assignment;
            else if constexpr (std::is_same_v<T, Reassignment>) return reassignment;
            else if constexpr (std::is_same_v<T, ExpressionStatement>) return expression_statement;
            else if constexpr (std::is_same_v<T, ReturnStatement>) return return_statement;
            else if constexpr (std::is_same_v<T, EnumDefinition>) return enum_definition;
            else if constexpr (std::is_same_v<T, Directive>) return directive;
            else if constexpr (std::is_same_v<T, ImportStatement>) return import_statement;
            else if constexpr (std::is_same_v<T, FunctionDefinition>) return function_definition;
            else if constexpr (std::is_same_v<T, StructDefinition>) return struct_definition;
            else if constexpr (std::is_same_v<T, TypeAliasDefinition>) return type_alias_definition;
            else if constexpr (std::is_same_v<T, ExtensionDefinition>) return extension_definition;
            else if constexpr (std::is_same_v<T, Program>) return program;
            else if constexpr (std::is_same_v<T, TypeAnnotation>) return type_annotation;
            else if constexpr (std::is_same_v<T, TupleTypeAnnotation>) return tuple_type_annotation;
        }

        template <typename T>
        constexpr const auto& get() const
        {
            if constexpr (std::is_same_v<T, Comment>) return comment;
            else if constexpr (std::is_same_v<T, CallArgument>) return call_argument;
            else if constexpr (std::is_same_v<T, Modifier>) return modifier;
            else if constexpr (std::is_same_v<T, FunctionParameter>) return function_parameter;
            else if constexpr (std::is_same_v<T, StructField>) return struct_field;
            else if constexpr (std::is_same_v<T, EnumCase>) return enum_case;
            else if constexpr (std::is_same_v<T, DictItem>) return dict_item;
            else if constexpr (std::is_same_v<T, SwitchCase>) return switch_case;
            else if constexpr (std::is_same_v<T, AssignmentTarget>) return assignment_target;
            else if constexpr (std::is_same_v<T, NumberLiteral>) return number_literal;
            else if constexpr (std::is_same_v<T, PercentageLiteral>) return percentage_literal;
            else if constexpr (std::is_same_v<T, StringLiteral>) return string_literal;
            else if constexpr (std::is_same_v<T, BooleanLiteral>) return boolean_literal;
            else if constexpr (std::is_same_v<T, IdentifierAccess>) return identifier_access;
            else if constexpr (std::is_same_v<T, SelfExpression>) return self_expression;
            else if constexpr (std::is_same_v<T, BinaryExpression>) return binary_expression;
            else if constexpr (std::is_same_v<T, UnaryExpression>) return unary_expression;
            else if constexpr (std::is_same_v<T, GroupingExpression>) return grouping_expression;
            else if constexpr (std::is_same_v<T, ConditionalExpression>) return conditional_expression;
            else if constexpr (std::is_same_v<T, FunctionCall>) return function_call;
            else if constexpr (std::is_same_v<T, DictLiteral>) return dict_literal;
            else if constexpr (std::is_same_v<T, TensorLiteral>) return tensor_literal;
            else if constexpr (std::is_same_v<T, TupleLiteral>) return tuple_literal;
            else if constexpr (std::is_same_v<T, BracketAccess>) return bracket_access;
            else if constexpr (std::is_same_v<T, DotAccess>) return dot_access;
            else if constexpr (std::is_same_v<T, SwitchExpression>) return switch_expression;
            else if constexpr (std::is_same_v<T, Assignment>) return assignment;
            else if constexpr (std::is_same_v<T, Reassignment>) return reassignment;
            else if constexpr (std::is_same_v<T, ExpressionStatement>) return expression_statement;
            else if constexpr (std::is_same_v<T, ReturnStatement>) return return_statement;
            else if constexpr (std::is_same_v<T, EnumDefinition>) return enum_definition;
            else if constexpr (std::is_same_v<T, Directive>) return directive;
            else if constexpr (std::is_same_v<T, ImportStatement>) return import_statement;
            else if constexpr (std::is_same_v<T, FunctionDefinition>) return function_definition;
            else if constexpr (std::is_same_v<T, StructDefinition>) return struct_definition;
            else if constexpr (std::is_same_v<T, TypeAliasDefinition>) return type_alias_definition;
            else if constexpr (std::is_same_v<T, ExtensionDefinition>) return extension_definition;
            else if constexpr (std::is_same_v<T, Program>) return program;
            else if constexpr (std::is_same_v<T, TypeAnnotation>) return type_annotation;
            else if constexpr (std::is_same_v<T, TupleTypeAnnotation>) return tuple_type_annotation;
        }
    };
}
