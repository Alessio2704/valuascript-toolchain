#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <utility>
#include <cstddef>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/utils/ast_builder.h"
#include "ast/factory/ast_factory_state.h"
#include "ast/factory/ast_factory_config.h"

namespace valuascript::compiler
{
    template <typename T>
    struct AstFactory;

    template <typename T>
    inline auto create_sample(int depth, const AstFactoryConfig& config = {})
    {
        return AstFactory<T>::create(depth, config);
    }

    template <typename T>
    inline auto sample_node(int depth, const AstFactoryConfig& config = {});

    inline ExprPtr sample_expr(int depth, const AstFactoryConfig& config = {});
    inline ExprPtr sample_expr_by_kind(ExpressionKind kind, int depth, const AstFactoryConfig& config = {});
    inline ExprPtr sample_reassignment_target_by_kind(ReassignmentTargetKind kind, int depth, const AstFactoryConfig& config = {});

    inline StmtPtr sample_stmt(int depth, const AstFactoryConfig& config = {});
    inline StmtPtr sample_stmt_by_kind(StatementKind kind, int depth, const AstFactoryConfig& config = {});

    inline TypeAnnPtr sample_type(int depth, const AstFactoryConfig& config = {});
    inline TypeAnnPtr sample_type_by_kind(TypeAnnotationKind kind, int depth, const AstFactoryConfig& config = {});

    inline CallArgument sample_call_arg(int depth, const AstFactoryConfig& config = {});
    inline Modifier sample_modifier(int depth, const AstFactoryConfig& config = {});
    inline FunctionParameter sample_param(int depth, const AstFactoryConfig& config = {});
    inline StructField sample_field(int depth, const AstFactoryConfig& config = {});
    inline EnumCase sample_enum_case(int depth, const AstFactoryConfig& config = {});
    inline DictItem sample_dict_item(int depth, const AstFactoryConfig& config = {});
    inline SwitchCase sample_switch_case(int depth, const AstFactoryConfig& config = {});
    inline AssignmentTarget sample_assignment_target(int depth, const AstFactoryConfig& config = {});

    inline std::vector<CallArgument> sample_call_args(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<Modifier> sample_modifiers(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<FunctionParameter> sample_params(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<StructField> sample_fields(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<EnumCase> sample_enum_cases(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<DictItem> sample_dict_items(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<SwitchCase> sample_switch_cases(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<AssignmentTarget> sample_assignment_targets(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<TypeAnnPtr> sample_type_vec(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<StmtPtr> sample_stmt_vec(size_t count, int depth, const AstFactoryConfig& config = {});
    inline std::vector<ExprPtr> sample_expr_vec(size_t count, int depth, const AstFactoryConfig& config = {});

    template <>
    struct AstFactory<Comment>
    {
        static Comment create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<Comment>();
            return AstBuilder::build<Comment>(c.text.value, factory_span(depth));
        }
    };

    template <>
    struct AstFactory<CallArgument>
    {
        static CallArgument create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<CallArgument>();
            return AstBuilder::build<CallArgument>(factory_name(c.name.prefix, depth), sample_expr(depth + 1, config), factory_span(depth));
        }
    };

    template <>
    struct AstFactory<Modifier>
    {
        static Modifier create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<Modifier>();
            return AstBuilder::build<Modifier>(
                factory_name(c.name.prefix, depth),
                sample_call_args(c.arguments.count, depth + 1, config),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<FunctionParameter>
    {
        static FunctionParameter create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<FunctionParameter>();
            auto def_expr = c.default_value.value.value_or(false)
                ? sample_expr(depth + 1, config)
                : nullptr;
            return AstBuilder::build<FunctionParameter>(
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_type(depth + 1, config),
                std::move(def_expr),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<StructField>
    {
        static StructField create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<StructField>();
            return AstBuilder::build<StructField>(
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_type(depth + 1, config),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<EnumCase>
    {
        static EnumCase create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<EnumCase>();
            auto val_expr = c.value.value.value_or(false)
                ? sample_expr(depth + 1, config)
                : nullptr;
            return AstBuilder::build<EnumCase>(
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                std::move(val_expr),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<DictItem>
    {
        static DictItem create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<DictItem>();
            return AstBuilder::build<DictItem>(
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.key.prefix, depth),
                sample_expr(depth + 1, config),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<SwitchCase>
    {
        static SwitchCase create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<SwitchCase>();
            std::vector<NodeName> ids;
            ids.reserve(c.identifiers.count);
            for (size_t i = 0; i < c.identifiers.count; ++i)
            {
                ids.push_back(factory_name("CaseId" + std::to_string(i), depth));
            }
            return AstBuilder::build<SwitchCase>(
                sample_modifiers(c.modifiers.count, depth + 1, config),
                std::move(ids),
                sample_expr(depth + 1, config),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<AssignmentTarget>
    {
        static AssignmentTarget create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<AssignmentTarget>();
            return AstBuilder::build<AssignmentTarget>(
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_type(depth + 1, config),
                factory_span(depth)
            );
        }
    };

    template <>
    struct AstFactory<NumberLiteral>
    {
        static std::unique_ptr<NumberLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<NumberLiteral>();
            return AstBuilder::build_with_span<NumberLiteral>(factory_span(depth), c.value.value);
        }
    };

    template <>
    struct AstFactory<PercentageLiteral>
    {
        static std::unique_ptr<PercentageLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<PercentageLiteral>();
            return AstBuilder::build_with_span<PercentageLiteral>(factory_span(depth), c.value.value);
        }
    };

    template <>
    struct AstFactory<StringLiteral>
    {
        static std::unique_ptr<StringLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<StringLiteral>();
            return AstBuilder::build_with_span<StringLiteral>(factory_span(depth), c.value.value);
        }
    };

    template <>
    struct AstFactory<BooleanLiteral>
    {
        static std::unique_ptr<BooleanLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<BooleanLiteral>();
            return AstBuilder::build_with_span<BooleanLiteral>(factory_span(depth), c.value.value);
        }
    };

    template <>
    struct AstFactory<IdentifierAccess>
    {
        static std::unique_ptr<IdentifierAccess> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<IdentifierAccess>();
            return AstBuilder::build_with_span<IdentifierAccess>(factory_span(depth), factory_name(c.name.prefix, depth));
        }
    };

    template <>
    struct AstFactory<SelfExpression>
    {
        static std::unique_ptr<SelfExpression> create(int depth, const AstFactoryConfig& /*config*/ = {})
        {
            return AstBuilder::build_with_span<SelfExpression>(factory_span(depth));
        }
    };

    template <>
    struct AstFactory<BinaryExpression>
    {
        static std::unique_ptr<BinaryExpression> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<BinaryExpression>();
            return AstBuilder::build_with_span<BinaryExpression>(
                factory_span(depth),
                sample_expr(depth + 1, config),
                c.op.op,
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<UnaryExpression>
    {
        static std::unique_ptr<UnaryExpression> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<UnaryExpression>();
            return AstBuilder::build_with_span<UnaryExpression>(
                factory_span(depth),
                c.op.op,
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<GroupingExpression>
    {
        static std::unique_ptr<GroupingExpression> create(int depth, const AstFactoryConfig& config = {})
        {
            return AstBuilder::build_with_span<GroupingExpression>(
                factory_span(depth),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<ConditionalExpression>
    {
        static std::unique_ptr<ConditionalExpression> create(int depth, const AstFactoryConfig& config = {})
        {
            return AstBuilder::build_with_span<ConditionalExpression>(
                factory_span(depth),
                sample_expr(depth + 1, config),
                sample_expr(depth + 1, config),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<FunctionCall>
    {
        static std::unique_ptr<FunctionCall> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<FunctionCall>();
            return AstBuilder::build_with_span<FunctionCall>(
                factory_span(depth),
                sample_expr(depth + 1, config),
                sample_call_args(c.arguments.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<DictLiteral>
    {
        static std::unique_ptr<DictLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<DictLiteral>();
            return AstBuilder::build_with_span<DictLiteral>(
                factory_span(depth),
                sample_dict_items(c.elements.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<TensorLiteral>
    {
        static std::unique_ptr<TensorLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<TensorLiteral>();
            return AstBuilder::build_with_span<TensorLiteral>(
                factory_span(depth),
                sample_expr_vec(c.elements.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<TupleLiteral>
    {
        static std::unique_ptr<TupleLiteral> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<TupleLiteral>();
            return AstBuilder::build_with_span<TupleLiteral>(
                factory_span(depth),
                sample_expr_vec(c.elements.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<BracketAccess>
    {
        static std::unique_ptr<BracketAccess> create(int depth, const AstFactoryConfig& config = {})
        {
            return AstBuilder::build_with_span<BracketAccess>(
                factory_span(depth),
                sample_expr(depth + 1, config),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<DotAccess>
    {
        static std::unique_ptr<DotAccess> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<DotAccess>();
            return AstBuilder::build_with_span<DotAccess>(
                factory_span(depth),
                sample_expr(depth + 1, config),
                factory_name(c.property_name.prefix, depth)
            );
        }
    };

    template <>
    struct AstFactory<SwitchExpression>
    {
        static std::unique_ptr<SwitchExpression> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<SwitchExpression>();
            auto def_expr = c.default_case.value.value_or(false)
                ? sample_expr(depth + 1, config)
                : nullptr;
            return AstBuilder::build_with_span<SwitchExpression>(
                factory_span(depth),
                sample_expr(depth + 1, config),
                sample_switch_cases(c.cases.count, depth + 1, config),
                sample_modifiers(c.default_modifiers.count, depth + 1, config),
                std::move(def_expr)
            );
        }
    };

    template <>
    struct AstFactory<Assignment>
    {
        static std::unique_ptr<Assignment> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<Assignment>();
            return AstBuilder::build_with_span<Assignment>(
                factory_span(depth),
                sample_assignment_targets(c.targets.count, depth + 1, config),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<Reassignment>
    {
        static std::unique_ptr<Reassignment> create(int depth, const AstFactoryConfig& config = {})
        {
            return AstBuilder::build_with_span<Reassignment>(
                factory_span(depth),
                sample_reassignment_target_by_kind(config.general.reassignment_target_kind, depth + 1, config),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<ExpressionStatement>
    {
        static std::unique_ptr<ExpressionStatement> create(int depth, const AstFactoryConfig& config = {})
        {
            return AstBuilder::build_with_span<ExpressionStatement>(
                factory_span(depth),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<ReturnStatement>
    {
        static std::unique_ptr<ReturnStatement> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<ReturnStatement>();
            return AstBuilder::build_with_span<ReturnStatement>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                sample_expr_vec(c.values.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<EnumDefinition>
    {
        static std::unique_ptr<EnumDefinition> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<EnumDefinition>();
            return AstBuilder::build_with_span<EnumDefinition>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_type(depth + 1, config),
                sample_enum_cases(c.cases.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<Directive>
    {
        static std::unique_ptr<Directive> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<Directive>();
            return AstBuilder::build_with_span<Directive>(
                factory_span(depth),
                factory_name(c.name.prefix, depth),
                sample_expr(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<ImportStatement>
    {
        static std::unique_ptr<ImportStatement> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<ImportStatement>();
            auto node = AstBuilder::build_with_span<ImportStatement>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.path.prefix, depth)
            );
            node->resolved_canonical_path = c.resolved_canonical_path.value;
            return node;
        }
    };

    template <>
    struct AstFactory<FunctionDefinition>
    {
        static std::unique_ptr<FunctionDefinition> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<FunctionDefinition>();
            return AstBuilder::build_with_span<FunctionDefinition>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_params(c.parameters.count, depth + 1, config),
                sample_type_vec(c.return_types.count, depth + 1, config),
                sample_stmt_vec(c.body.count, depth + 1, config),
                c.docstring.value
            );
        }
    };

    template <>
    struct AstFactory<StructDefinition>
    {
        static std::unique_ptr<StructDefinition> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<StructDefinition>();
            return AstBuilder::build_with_span<StructDefinition>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_fields(c.fields.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<TypeAliasDefinition>
    {
        static std::unique_ptr<TypeAliasDefinition> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<TypeAliasDefinition>();
            return AstBuilder::build_with_span<TypeAliasDefinition>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                factory_name(c.name.prefix, depth),
                sample_type(depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<ExtensionDefinition>
    {
        static std::unique_ptr<ExtensionDefinition> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<ExtensionDefinition>();
            auto node = AstBuilder::build_with_span<ExtensionDefinition>(
                factory_span(depth),
                sample_modifiers(c.modifiers.count, depth + 1, config),
                sample_type(depth + 1, config)
            );
            node->execution_steps = sample_stmt_vec(c.execution_steps.count, depth + 1, config);
            for (size_t i = 0; i < c.function_definitions.count; ++i)
                node->function_definitions.push_back(create_sample<FunctionDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.struct_definitions.count; ++i)
                node->struct_definitions.push_back(create_sample<StructDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.enum_definitions.count; ++i)
                node->enum_definitions.push_back(create_sample<EnumDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.type_aliases.count; ++i)
                node->type_aliases.push_back(create_sample<TypeAliasDefinition>(depth + 1, config));
            return node;
        }
    };

    template <>
    struct AstFactory<Program>
    {
        static std::unique_ptr<Program> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<Program>();
            auto node = AstBuilder::build_with_span<Program>(factory_span(depth));
            node->execution_steps = sample_stmt_vec(c.execution_steps.count, depth + 1, config);
            for (size_t i = 0; i < c.function_definitions.count; ++i)
                node->function_definitions.push_back(create_sample<FunctionDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.struct_definitions.count; ++i)
                node->struct_definitions.push_back(create_sample<StructDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.enum_definitions.count; ++i)
                node->enum_definitions.push_back(create_sample<EnumDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.type_aliases.count; ++i)
                node->type_aliases.push_back(create_sample<TypeAliasDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.extension_definitions.count; ++i)
                node->extension_definitions.push_back(create_sample<ExtensionDefinition>(depth + 1, config));
            for (size_t i = 0; i < c.import_statements.count; ++i)
                node->import_statements.push_back(create_sample<ImportStatement>(depth + 1, config));
            for (size_t i = 0; i < c.directives.count; ++i)
                node->directives.push_back(create_sample<Directive>(depth + 1, config));
            for (size_t i = 0; i < c.comments.count; ++i)
                node->comments.push_back(create_sample<Comment>(depth + 1, config));
            return node;
        }
    };

    template <>
    struct AstFactory<TypeAnnotation>
    {
        static std::unique_ptr<TypeAnnotation> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<TypeAnnotation>();
            return AstBuilder::build_with_span<TypeAnnotation>(
                factory_span(depth),
                factory_name(c.name.prefix, depth),
                sample_type_vec(c.generic_args.count, depth + 1, config)
            );
        }
    };

    template <>
    struct AstFactory<TupleTypeAnnotation>
    {
        static std::unique_ptr<TupleTypeAnnotation> create(int depth, const AstFactoryConfig& config = {})
        {
            const auto& c = config.get<TupleTypeAnnotation>();
            auto node = AstBuilder::build_with_span<TupleTypeAnnotation>(
                factory_span(depth),
                sample_type_vec(c.element_types.count, depth + 1, config)
            );
            if (!c.name.prefix.empty())
            {
                node->name = factory_name(c.name.prefix, depth);
            }
            if (c.generic_args.count > 0)
            {
                node->generic_args = sample_type_vec(c.generic_args.count, depth + 1, config);
            }
            return node;
        }
    };

    template <typename T>
    inline auto sample_nodes(size_t count, int depth, const AstFactoryConfig& config = {})
    {
        if constexpr (std::same_as<T, Expression>)
        {
            return sample_expr_vec(count, depth, config);
        }
        else if constexpr (std::same_as<T, Statement>)
        {
            return sample_stmt_vec(count, depth, config);
        }
        else
        {
            std::vector<decltype(create_sample<T>(depth + 1, config))> vec;
            vec.reserve(count);
            for (size_t i = 0; i < count; ++i) vec.push_back(create_sample<T>(depth + 1, config));
            return vec;
        }
    }

    inline std::vector<TypeAnnPtr> sample_type_vec(size_t count, int depth, const AstFactoryConfig& config)
    {
        std::vector<TypeAnnPtr> vec;
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) vec.push_back(sample_type(depth + 1, config));
        return vec;
    }

    inline std::vector<StmtPtr> sample_stmt_vec(size_t count, int depth, const AstFactoryConfig& config)
    {
        std::vector<StmtPtr> vec;
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) vec.push_back(sample_stmt(depth + 1, config));
        return vec;
    }

    inline std::vector<ExprPtr> sample_expr_vec(size_t count, int depth, const AstFactoryConfig& config)
    {
        std::vector<ExprPtr> vec;
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) vec.push_back(sample_expr(depth + 1, config));
        return vec;
    }

    inline std::vector<CallArgument> sample_call_args(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<CallArgument>(count, depth, config);
    }

    inline std::vector<Modifier> sample_modifiers(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<Modifier>(count, depth, config);
    }

    inline std::vector<FunctionParameter> sample_params(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<FunctionParameter>(count, depth, config);
    }

    inline std::vector<StructField> sample_fields(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<StructField>(count, depth, config);
    }

    inline std::vector<EnumCase> sample_enum_cases(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<EnumCase>(count, depth, config);
    }

    inline std::vector<DictItem> sample_dict_items(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<DictItem>(count, depth, config);
    }

    inline std::vector<SwitchCase> sample_switch_cases(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<SwitchCase>(count, depth, config);
    }

    inline std::vector<AssignmentTarget> sample_assignment_targets(size_t count, int depth, const AstFactoryConfig& config)
    {
        return sample_nodes<AssignmentTarget>(count, depth, config);
    }

    inline CallArgument sample_call_arg(int depth, const AstFactoryConfig& config)
    {
        return create_sample<CallArgument>(depth, config);
    }

    inline Modifier sample_modifier(int depth, const AstFactoryConfig& config)
    {
        return create_sample<Modifier>(depth, config);
    }

    inline FunctionParameter sample_param(int depth, const AstFactoryConfig& config)
    {
        return create_sample<FunctionParameter>(depth, config);
    }

    inline StructField sample_field(int depth, const AstFactoryConfig& config)
    {
        return create_sample<StructField>(depth, config);
    }

    inline EnumCase sample_enum_case(int depth, const AstFactoryConfig& config)
    {
        return create_sample<EnumCase>(depth, config);
    }

    inline DictItem sample_dict_item(int depth, const AstFactoryConfig& config)
    {
        return create_sample<DictItem>(depth, config);
    }

    inline SwitchCase sample_switch_case(int depth, const AstFactoryConfig& config)
    {
        return create_sample<SwitchCase>(depth, config);
    }

    inline AssignmentTarget sample_assignment_target(int depth, const AstFactoryConfig& config)
    {
        return create_sample<AssignmentTarget>(depth, config);
    }

    template <typename T>
        requires (valuascript::shared::tuple_contains_type_v<T, AllExpressionNodeTypes>)
    inline ExprPtr sample_expr_as(int depth, const AstFactoryConfig& config = {})
    {
        return create_sample<T>(depth, config);
    }

    template <typename T>
        requires (valuascript::shared::tuple_contains_type_v<T, AllStatementNodeTypes>)
    inline StmtPtr sample_stmt_as(int depth, const AstFactoryConfig& config = {})
    {
        return create_sample<T>(depth, config);
    }

    template <typename T>
        requires (valuascript::shared::tuple_contains_type_v<T, AllTypeAnnotationNodeTypes>)
    inline TypeAnnPtr sample_type_as(int depth, const AstFactoryConfig& config = {})
    {
        return create_sample<T>(depth, config);
    }

    inline ExprPtr sample_reassignment_target_by_kind(ReassignmentTargetKind kind, int depth, const AstFactoryConfig& config)
    {
        ExprPtr result = nullptr;
        NodeDispatcher<AllReassignmentTargetNodeTypes>::dispatch(kind, [&]<typename T>()
        {
            result = create_sample<T>(depth, config);
        });
        return result;
    }

    inline ExprPtr sample_expr_by_kind(ExpressionKind kind, int depth, const AstFactoryConfig& config)
    {
        ExprPtr result = nullptr;
        NodeDispatcher<AllExpressionNodeTypes>::dispatch(kind, [&]<typename T>()
        {
            result = create_sample<T>(depth, config);
        });
        return result;
    }

    inline StmtPtr sample_stmt_by_kind(StatementKind kind, int depth, const AstFactoryConfig& config)
    {
        StmtPtr result = nullptr;
        NodeDispatcher<AllStatementNodeTypes>::dispatch(kind, [&]<typename T>()
        {
            result = create_sample<T>(depth, config);
        });
        return result;
    }

    inline TypeAnnPtr sample_type_by_kind(TypeAnnotationKind kind, int depth, const AstFactoryConfig& config)
    {
        TypeAnnPtr result = nullptr;
        NodeDispatcher<AllTypeAnnotationNodeTypes>::dispatch(kind, [&]<typename T>()
        {
            result = create_sample<T>(depth, config);
        });
        return result;
    }

    template <typename Category>
    inline auto sample_node_by_kind(AstKind kind, int depth, const AstFactoryConfig& config = {})
    {
        if constexpr (std::same_as<Category, Expression>)
        {
            return sample_expr_by_kind(ExpressionKind{kind}, depth, config);
        }
        else if constexpr (std::same_as<Category, Statement>)
        {
            return sample_stmt_by_kind(StatementKind{kind}, depth, config);
        }
        else if constexpr (std::same_as<Category, TypeAnnotation>)
        {
            return sample_type_by_kind(TypeAnnotationKind{kind}, depth, config);
        }
    }

    template <typename T>
    inline auto sample_node_as(int depth, const AstFactoryConfig& config = {})
    {
        return create_sample<T>(depth, config);
    }

    inline ExprPtr sample_expr(int depth, const AstFactoryConfig& config)
    {
        if (depth >= config.general.max_expression_depth)
        {
            return create_sample<NumberLiteral>(depth, config);
        }
        return sample_expr_by_kind(config.general.expression_kind, depth, config);
    }

    inline StmtPtr sample_stmt(int depth, const AstFactoryConfig& config)
    {
        return sample_stmt_by_kind(config.general.statement_kind, depth, config);
    }

    inline TypeAnnPtr sample_type(int depth, const AstFactoryConfig& config)
    {
        if (depth >= config.general.max_type_depth)
        {
            return AstBuilder::build_with_span<TypeAnnotation>(factory_span(depth), factory_name("LeafType", depth),
                                                               std::vector<TypeAnnPtr>{});
        }
        return sample_type_by_kind(config.general.type_kind, depth, config);
    }

    template <typename T>
    inline auto sample_node(int depth, const AstFactoryConfig& config)
    {
        if constexpr (std::same_as<T, Expression>)
        {
            return sample_expr(depth, config);
        }
        else if constexpr (std::same_as<T, Statement>)
        {
            return sample_stmt(depth, config);
        }
        else
        {
            return create_sample<T>(depth, config);
        }
    }

    inline std::unique_ptr<Program> create_sample_program(int depth = 0, const AstFactoryConfig& config = {})
    {
        return create_sample<Program>(depth, config);
    }
}
