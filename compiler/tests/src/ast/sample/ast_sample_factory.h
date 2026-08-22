#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <cstddef>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast_sample_state.h"

namespace valuascript::compiler::test
{

    inline ExprPtr sample_expr(int depth = 0);
    inline StmtPtr sample_stmt(int depth = 0);
    inline TypeAnnPtr sample_type(int depth = 0);

    inline CallArgument sample_call_arg(int depth = 0);
    inline Modifier sample_modifier(int depth = 0);
    inline FunctionParameter sample_param(int depth = 0);
    inline StructField sample_field(int depth = 0);
    inline EnumCase sample_enum_case(int depth = 0);
    inline DictItem sample_dict_item(int depth = 0);
    inline SwitchCase sample_switch_case(int depth = 0);
    inline AssignmentTarget sample_assignment_target(int depth = 0);

    inline std::vector<CallArgument> sample_call_args(int count = 2, int depth = 0)
    {
        std::vector<CallArgument> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_call_arg(depth + 1));
        return vec;
    }

    inline std::vector<Modifier> sample_modifiers(int count = 2, int depth = 0)
    {
        std::vector<Modifier> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_modifier(depth + 1));
        return vec;
    }

    inline std::vector<FunctionParameter> sample_params(int count = 2, int depth = 0)
    {
        std::vector<FunctionParameter> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_param(depth + 1));
        return vec;
    }

    inline std::vector<StructField> sample_fields(int count = 2, int depth = 0)
    {
        std::vector<StructField> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_field(depth + 1));
        return vec;
    }

    inline std::vector<EnumCase> sample_enum_cases(int count = 2, int depth = 0)
    {
        std::vector<EnumCase> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_enum_case(depth + 1));
        return vec;
    }

    inline std::vector<DictItem> sample_dict_items(int count = 2, int depth = 0)
    {
        std::vector<DictItem> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_dict_item(depth + 1));
        return vec;
    }

    inline std::vector<SwitchCase> sample_switch_cases(int count = 2, int depth = 0)
    {
        std::vector<SwitchCase> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_switch_case(depth + 1));
        return vec;
    }

    inline std::vector<AssignmentTarget> sample_assignment_targets(int count = 3, int depth = 0)
    {
        std::vector<AssignmentTarget> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_assignment_target(depth + 1));
        return vec;
    }

    inline std::vector<TypeAnnPtr> sample_type_vec(int count = 2, int depth = 0)
    {
        std::vector<TypeAnnPtr> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_type(depth + 1));
        return vec;
    }

    inline std::vector<StmtPtr> sample_stmt_vec(int count = 2, int depth = 0)
    {
        std::vector<StmtPtr> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_stmt(depth + 1));
        return vec;
    }

    inline std::vector<ExprPtr> sample_expr_vec(int count = 2, int depth = 0)
    {
        std::vector<ExprPtr> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(sample_expr(depth + 1));
        return vec;
    }

    inline ExprPtr sample_expr(int depth)
    {
        if (depth >= 2)
        {
            auto lit = std::make_unique<NumberLiteral>(std::to_string(AstSampleState::next_id()));
            lit->span = sample_span(depth);
            return lit;
        }
        if (depth == 1)
        {
            auto id_acc = std::make_unique<IdentifierAccess>(sample_name("id_leaf", depth));
            id_acc->span = sample_span(depth);
            return id_acc;
        }
        auto bin = std::make_unique<BinaryExpression>(sample_expr(depth + 1), TokenType::Plus, sample_expr(depth + 2));
        bin->span = sample_span(depth);
        return bin;
    }

    inline StmtPtr sample_stmt(int depth)
    {
        if (depth >= 2)
        {
            auto ret = std::make_unique<ReturnStatement>(sample_modifiers(1, depth + 1), sample_expr_vec(2, depth + 1));
            ret->span = sample_span(depth);
            return ret;
        }
        auto expr_stmt = std::make_unique<ExpressionStatement>(sample_expr(depth + 1));
        expr_stmt->span = sample_span(depth);
        return expr_stmt;
    }

    inline TypeAnnPtr sample_type(int depth)
    {
        if (depth >= 3)
        {
            auto t = std::make_unique<TypeAnnotation>(sample_name("LeafType", depth));
            t->span = sample_span(depth);
            return t;
        }
        if (depth == 2)
        {
            std::vector<TypeAnnPtr> elems;
            elems.push_back(std::make_unique<TypeAnnotation>(sample_name("TupleElemA", depth + 1)));
            elems.back()->span = sample_span(depth + 1);
            elems.push_back(std::make_unique<TypeAnnotation>(sample_name("TupleElemB", depth + 1)));
            elems.back()->span = sample_span(depth + 1);
            auto tuple_t = std::make_unique<TupleTypeAnnotation>(std::move(elems));
            tuple_t->span = sample_span(depth);
            return tuple_t;
        }
        if (depth == 1)
        {
            std::vector<TypeAnnPtr> args;
            args.push_back(sample_type(depth + 1));
            args.push_back(sample_type(depth + 2));
            auto gen_t = std::make_unique<TypeAnnotation>(sample_name("GenericType", depth), std::move(args));
            gen_t->span = sample_span(depth);
            return gen_t;
        }
        std::vector<TypeAnnPtr> outer_elems;
        outer_elems.push_back(sample_type(1));
        outer_elems.push_back(sample_type(2));
        auto nested_tuple = std::make_unique<TupleTypeAnnotation>(std::move(outer_elems));
        nested_tuple->span = sample_span(depth);
        return nested_tuple;
    }

    inline CallArgument sample_call_arg(int depth)
    {
        return CallArgument(sample_name("arg", depth), sample_expr(depth + 1), sample_span(depth));
    }

    inline Modifier sample_modifier(int depth)
    {
        return Modifier(sample_name("mod", depth), depth < 2 ? sample_call_args(2, depth + 1) : std::vector<CallArgument>{}, sample_span(depth));
    }

    inline FunctionParameter sample_param(int depth)
    {
        return FunctionParameter(sample_modifiers(2, depth + 1), sample_name("param", depth), sample_type(depth + 1), sample_expr(depth + 1), sample_span(depth));
    }

    inline StructField sample_field(int depth)
    {
        return StructField(sample_modifiers(2, depth + 1), sample_name("field", depth), sample_type(depth + 1), sample_span(depth));
    }

    inline EnumCase sample_enum_case(int depth)
    {
        return EnumCase(sample_modifiers(2, depth + 1), sample_name("Case", depth), sample_expr(depth + 1), sample_span(depth));
    }

    inline DictItem sample_dict_item(int depth)
    {
        return DictItem(sample_modifiers(2, depth + 1), sample_name("key", depth), sample_expr(depth + 1), sample_span(depth));
    }

    inline SwitchCase sample_switch_case(int depth)
    {
        std::vector<NodeName> ids{sample_name("CaseIdA", depth), sample_name("CaseIdB", depth)};
        return SwitchCase(sample_modifiers(2, depth + 1), std::move(ids), sample_expr(depth + 1), sample_span(depth));
    }

    inline AssignmentTarget sample_assignment_target(int depth)
    {
        return AssignmentTarget(sample_modifiers(2, depth + 1), sample_name("target", depth), sample_type(depth + 1), sample_span(depth));
    }

    template <typename T>
    struct AstSampleFactory;

    template <typename T>
    inline auto create_sample(int depth = 0)
    {
        return AstSampleFactory<T>::create(depth);
    }

    template <>
    struct AstSampleFactory<Comment>
    {
        static Comment create(int depth = 0)
        {
            return Comment("comment_" + std::to_string(AstSampleState::next_id()), sample_span(depth));
        }
    };

    template <>
    struct AstSampleFactory<CallArgument>
    {
        static CallArgument create(int depth = 0)
        {
            return sample_call_arg(depth);
        }
    };

    template <>
    struct AstSampleFactory<Modifier>
    {
        static Modifier create(int depth = 0)
        {
            return sample_modifier(depth);
        }
    };

    template <>
    struct AstSampleFactory<FunctionParameter>
    {
        static FunctionParameter create(int depth = 0)
        {
            return sample_param(depth);
        }
    };

    template <>
    struct AstSampleFactory<StructField>
    {
        static StructField create(int depth = 0)
        {
            return sample_field(depth);
        }
    };

    template <>
    struct AstSampleFactory<EnumCase>
    {
        static EnumCase create(int depth = 0)
        {
            return sample_enum_case(depth);
        }
    };

    template <>
    struct AstSampleFactory<DictItem>
    {
        static DictItem create(int depth = 0)
        {
            return sample_dict_item(depth);
        }
    };

    template <>
    struct AstSampleFactory<SwitchCase>
    {
        static SwitchCase create(int depth = 0)
        {
            return sample_switch_case(depth);
        }
    };

    template <>
    struct AstSampleFactory<AssignmentTarget>
    {
        static AssignmentTarget create(int depth = 0)
        {
            return sample_assignment_target(depth);
        }
    };

    template <>
    struct AstSampleFactory<NumberLiteral>
    {
        static std::unique_ptr<NumberLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<NumberLiteral>(std::to_string(AstSampleState::next_id()) + ".75");
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<PercentageLiteral>
    {
        static std::unique_ptr<PercentageLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<PercentageLiteral>(std::to_string(AstSampleState::next_id() % 100) + ".5%");
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<StringLiteral>
    {
        static std::unique_ptr<StringLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<StringLiteral>("sample_string_" + std::to_string(AstSampleState::next_id()));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<BooleanLiteral>
    {
        static std::unique_ptr<BooleanLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<BooleanLiteral>(AstSampleState::next_id() % 2 == 0);
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<IdentifierAccess>
    {
        static std::unique_ptr<IdentifierAccess> create(int depth = 0)
        {
            auto node = std::make_unique<IdentifierAccess>(sample_name("identifier", depth));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<SelfExpression>
    {
        static std::unique_ptr<SelfExpression> create(int depth = 0)
        {
            auto node = std::make_unique<SelfExpression>();
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<BinaryExpression>
    {
        static std::unique_ptr<BinaryExpression> create(int depth = 0)
        {
            auto node = std::make_unique<BinaryExpression>(sample_expr(depth + 1), TokenType::Plus, sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<UnaryExpression>
    {
        static std::unique_ptr<UnaryExpression> create(int depth = 0)
        {
            auto node = std::make_unique<UnaryExpression>(TokenType::Minus, sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<GroupingExpression>
    {
        static std::unique_ptr<GroupingExpression> create(int depth = 0)
        {
            auto node = std::make_unique<GroupingExpression>(sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<ConditionalExpression>
    {
        static std::unique_ptr<ConditionalExpression> create(int depth = 0)
        {
            auto node = std::make_unique<ConditionalExpression>(sample_expr(depth + 1), sample_expr(depth + 1), sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<FunctionCall>
    {
        static std::unique_ptr<FunctionCall> create(int depth = 0)
        {
            auto node = std::make_unique<FunctionCall>(sample_expr(depth + 1), sample_call_args(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<DictLiteral>
    {
        static std::unique_ptr<DictLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<DictLiteral>(sample_dict_items(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<TensorLiteral>
    {
        static std::unique_ptr<TensorLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<TensorLiteral>(sample_expr_vec(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<TupleLiteral>
    {
        static std::unique_ptr<TupleLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<TupleLiteral>(sample_expr_vec(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<BracketAccess>
    {
        static std::unique_ptr<BracketAccess> create(int depth = 0)
        {
            auto node = std::make_unique<BracketAccess>(sample_expr(depth + 1), sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<DotAccess>
    {
        static std::unique_ptr<DotAccess> create(int depth = 0)
        {
            auto node = std::make_unique<DotAccess>(sample_expr(depth + 1), sample_name("prop", depth));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<SwitchExpression>
    {
        static std::unique_ptr<SwitchExpression> create(int depth = 0)
        {
            auto node = std::make_unique<SwitchExpression>(
                sample_expr(depth + 1),
                sample_switch_cases(3, depth + 1),
                sample_modifiers(2, depth + 1),
                sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<Assignment>
    {
        static std::unique_ptr<Assignment> create(int depth = 0)
        {
            auto node = std::make_unique<Assignment>(sample_assignment_targets(3, depth + 1), sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<Reassignment>
    {
        static std::unique_ptr<Reassignment> create(int depth = 0)
        {
            auto node = std::make_unique<Reassignment>(sample_expr(depth + 1), sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<ExpressionStatement>
    {
        static std::unique_ptr<ExpressionStatement> create(int depth = 0)
        {
            auto node = std::make_unique<ExpressionStatement>(sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<ReturnStatement>
    {
        static std::unique_ptr<ReturnStatement> create(int depth = 0)
        {
            auto node = std::make_unique<ReturnStatement>(sample_modifiers(2, depth + 1), sample_expr_vec(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<EnumDefinition>
    {
        static std::unique_ptr<EnumDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<EnumDefinition>(
                sample_modifiers(2, depth + 1),
                sample_name("EnumDef", depth),
                sample_type(depth + 1),
                sample_enum_cases(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<Directive>
    {
        static std::unique_ptr<Directive> create(int depth = 0)
        {
            auto node = std::make_unique<Directive>(sample_name("directive_name", depth), sample_expr(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<ImportStatement>
    {
        static std::unique_ptr<ImportStatement> create(int depth = 0)
        {
            auto node = std::make_unique<ImportStatement>(sample_modifiers(2, depth + 1), sample_name("import.module.path", depth));
            node->resolved_canonical_path = "/resolved/path/module_" + std::to_string(AstSampleState::next_id()) + ".vs";
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<FunctionDefinition>
    {
        static std::unique_ptr<FunctionDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<FunctionDefinition>(
                sample_modifiers(2, depth + 1),
                sample_name("func_name", depth),
                sample_params(3, depth + 1),
                sample_type_vec(2, depth + 1),
                sample_stmt_vec(3, depth + 1),
                std::make_optional<std::string>("Docstring for sample function"));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<StructDefinition>
    {
        static std::unique_ptr<StructDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<StructDefinition>(
                sample_modifiers(2, depth + 1),
                sample_name("StructName", depth),
                sample_fields(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<TypeAliasDefinition>
    {
        static std::unique_ptr<TypeAliasDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<TypeAliasDefinition>(
                sample_modifiers(2, depth + 1),
                sample_name("AliasName", depth),
                sample_type(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<ExtensionDefinition>
    {
        static std::unique_ptr<ExtensionDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<ExtensionDefinition>(
                sample_modifiers(2, depth + 1),
                sample_type(depth + 1));
            node->execution_steps = sample_stmt_vec(2, depth + 1);
            node->function_definitions.push_back(create_sample<FunctionDefinition>(depth + 1));
            node->struct_definitions.push_back(create_sample<StructDefinition>(depth + 1));
            node->enum_definitions.push_back(create_sample<EnumDefinition>(depth + 1));
            node->type_aliases.push_back(create_sample<TypeAliasDefinition>(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<Program>
    {
        static std::unique_ptr<Program> create(int depth = 0)
        {
            auto node = std::make_unique<Program>();
            node->comments.push_back(create_sample<Comment>(depth + 1));
            node->comments.push_back(create_sample<Comment>(depth + 1));
            node->import_statements.push_back(create_sample<ImportStatement>(depth + 1));
            node->directives.push_back(create_sample<Directive>(depth + 1));
            node->execution_steps = sample_stmt_vec(2, depth + 1);
            node->function_definitions.push_back(create_sample<FunctionDefinition>(depth + 1));
            node->struct_definitions.push_back(create_sample<StructDefinition>(depth + 1));
            node->enum_definitions.push_back(create_sample<EnumDefinition>(depth + 1));
            node->type_aliases.push_back(create_sample<TypeAliasDefinition>(depth + 1));
            node->extension_definitions.push_back(create_sample<ExtensionDefinition>(depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<TypeAnnotation>
    {
        static std::unique_ptr<TypeAnnotation> create(int depth = 0)
        {
            auto node = std::make_unique<TypeAnnotation>(sample_name("TypeAnn", depth), sample_type_vec(2, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    template <>
    struct AstSampleFactory<TupleTypeAnnotation>
    {
        static std::unique_ptr<TupleTypeAnnotation> create(int depth = 0)
        {
            auto node = std::make_unique<TupleTypeAnnotation>(sample_type_vec(3, depth + 1));
            node->span = sample_span(depth);
            return node;
        }
    };

    inline std::unique_ptr<Program> create_sample_program(int depth = 0)
    {
        return create_sample<Program>(depth);
    }
}
