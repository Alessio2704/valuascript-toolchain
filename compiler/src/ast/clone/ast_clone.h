#pragma once

#include <memory>
#include <vector>
#include <concepts>
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_concepts.h"
#include "ast/equality/ast_equality.h"
#include "ast/equality/ast_disjoint.h"

namespace valuascript::compiler
{
    template <AstElement T>
    [[nodiscard]] inline std::unique_ptr<T> clone_node(const T* node);

    template <AstElement T>
    [[nodiscard]] inline std::unique_ptr<T> clone_node(const std::unique_ptr<T>& ptr);

    template <typename T>
    [[nodiscard]] inline std::vector<T> clone_nodes(const std::vector<T>& vec);

    template <AstElement T>
    [[nodiscard]] inline std::vector<std::unique_ptr<T>> clone_nodes(const std::vector<std::unique_ptr<T>>& vec);

    [[nodiscard]] inline Comment clone_node(const Comment& comment);
    [[nodiscard]] inline CallArgument clone_node(const CallArgument& arg);
    [[nodiscard]] inline Modifier clone_node(const Modifier& mod);
    [[nodiscard]] inline FunctionParameter clone_node(const FunctionParameter& param);
    [[nodiscard]] inline StructField clone_node(const StructField& field);
    [[nodiscard]] inline EnumCase clone_node(const EnumCase& item);
    [[nodiscard]] inline DictItem clone_node(const DictItem& item);
    [[nodiscard]] inline SwitchCase clone_node(const SwitchCase& item);
    [[nodiscard]] inline AssignmentTarget clone_node(const AssignmentTarget& target);

    namespace detail
    {
        [[nodiscard]] inline ExprPtr clone_expression(const Expression* expr)
        {
            if (!expr) return nullptr;

            switch (expr->kind)
            {
            case AstKind::NumberLiteral:
                {
                    auto* e = static_cast<const NumberLiteral*>(expr);
                    auto copy = std::make_unique<NumberLiteral>(e->value);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::PercentageLiteral:
                {
                    auto* e = static_cast<const PercentageLiteral*>(expr);
                    auto copy = std::make_unique<PercentageLiteral>(e->value);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::StringLiteral:
                {
                    auto* e = static_cast<const StringLiteral*>(expr);
                    auto copy = std::make_unique<StringLiteral>(e->value);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::BooleanLiteral:
                {
                    auto* e = static_cast<const BooleanLiteral*>(expr);
                    auto copy = std::make_unique<BooleanLiteral>(e->value);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::IdentifierAccess:
                {
                    auto* e = static_cast<const IdentifierAccess*>(expr);
                    auto copy = std::make_unique<IdentifierAccess>(e->name);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::SelfExpression:
                {
                    auto* e = static_cast<const SelfExpression*>(expr);
                    auto copy = std::make_unique<SelfExpression>();
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::UnaryExpression:
                {
                    auto* e = static_cast<const UnaryExpression*>(expr);
                    auto copy = std::make_unique<UnaryExpression>(
                        e->op, clone_node(e->right.get()));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::BinaryExpression:
                {
                    auto* e = static_cast<const BinaryExpression*>(expr);
                    auto copy = std::make_unique<BinaryExpression>(
                        clone_node(e->left.get()), e->op, clone_node(e->right.get()));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::GroupingExpression:
                {
                    auto* e = static_cast<const GroupingExpression*>(expr);
                    auto copy = std::make_unique<GroupingExpression>(
                        clone_node(e->expression.get()));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::ConditionalExpression:
                {
                    auto* e = static_cast<const ConditionalExpression*>(expr);
                    auto copy = std::make_unique<ConditionalExpression>(
                        clone_node(e->condition.get()),
                        clone_node(e->then_branch.get()),
                        clone_node(e->else_branch.get()));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::BracketAccess:
                {
                    auto* e = static_cast<const BracketAccess*>(expr);
                    auto copy = std::make_unique<BracketAccess>(
                        clone_node(e->target.get()), clone_node(e->index.get()));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::DotAccess:
                {
                    auto* e = static_cast<const DotAccess*>(expr);
                    auto copy = std::make_unique<DotAccess>(
                        clone_node(e->target.get()), e->property_name);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::TupleLiteral:
                {
                    auto* e = static_cast<const TupleLiteral*>(expr);
                    auto copy = std::make_unique<TupleLiteral>(clone_nodes(e->elements));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::TensorLiteral:
                {
                    auto* e = static_cast<const TensorLiteral*>(expr);
                    auto copy = std::make_unique<TensorLiteral>(clone_nodes(e->elements));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::FunctionCall:
                {
                    auto* e = static_cast<const FunctionCall*>(expr);
                    auto copy = std::make_unique<FunctionCall>(
                        clone_node(e->target.get()), clone_nodes(e->arguments));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::DictLiteral:
                {
                    auto* e = static_cast<const DictLiteral*>(expr);
                    auto copy = std::make_unique<DictLiteral>(clone_nodes(e->elements));
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::SwitchExpression:
                {
                    auto* e = static_cast<const SwitchExpression*>(expr);
                    auto copy = std::make_unique<SwitchExpression>(
                        clone_node(e->target.get()),
                        clone_nodes(e->cases),
                        clone_nodes(e->default_modifiers),
                        clone_node(e->default_case.get()));
                    copy->span = e->span;
                    return copy;
                }
            default:
                return nullptr;
            }
        }

        [[nodiscard]] inline StmtPtr clone_statement(const Statement* stmt)
        {
            if (!stmt) return nullptr;

            switch (stmt->kind)
            {
            case AstKind::Assignment:
                {
                    auto* s = static_cast<const Assignment*>(stmt);
                    auto copy = std::make_unique<Assignment>(
                        clone_nodes(s->targets), clone_node(s->value.get()));
                    copy->span = s->span;
                    return copy;
                }
            case AstKind::Reassignment:
                {
                    auto* s = static_cast<const Reassignment*>(stmt);
                    auto copy = std::make_unique<Reassignment>(
                        clone_node(s->target.get()), clone_node(s->value.get()));
                    copy->span = s->span;
                    return copy;
                }
            case AstKind::ExpressionStatement:
                {
                    auto* s = static_cast<const ExpressionStatement*>(stmt);
                    auto copy = std::make_unique<ExpressionStatement>(
                        clone_node(s->expr.get()));
                    copy->span = s->span;
                    return copy;
                }
            case AstKind::ReturnStatement:
                {
                    auto* s = static_cast<const ReturnStatement*>(stmt);
                    auto copy = std::make_unique<ReturnStatement>(
                        clone_nodes(s->modifiers), clone_nodes(s->values));
                    copy->span = s->span;
                    return copy;
                }
            case AstKind::EnumDefinition:
                {
                    auto* s = static_cast<const EnumDefinition*>(stmt);
                    auto copy = std::make_unique<EnumDefinition>(
                        clone_nodes(s->modifiers),
                        s->name,
                        clone_node(s->underlying_type.get()),
                        clone_nodes(s->cases));
                    copy->span = s->span;
                    return copy;
                }
            default:
                return nullptr;
            }
        }

        [[nodiscard]] inline TypeAnnPtr clone_type_annotation(const TypeAnnotation* type)
        {
            if (!type) return nullptr;

            switch (type->kind)
            {
            case AstKind::TupleTypeAnnotation:
                {
                    auto* t = static_cast<const TupleTypeAnnotation*>(type);
                    auto copy = std::make_unique<TupleTypeAnnotation>(
                        clone_nodes(t->element_types));
                    copy->span = t->span;
                    return copy;
                }
            case AstKind::TypeAnnotation:
                {
                    auto* t = static_cast<const TypeAnnotation*>(type);
                    auto copy = std::make_unique<TypeAnnotation>(
                        t->name, clone_nodes(t->generic_args));
                    copy->span = t->span;
                    return copy;
                }
            default:
                return nullptr;
            }
        }

        [[nodiscard]] inline std::unique_ptr<AstNode> clone_ast_node(const AstNode* node)
        {
            if (!node) return nullptr;

            switch (node->kind)
            {
            case AstKind::Directive:
                {
                    auto* d = static_cast<const Directive*>(node);
                    auto copy = std::make_unique<Directive>(
                        d->name, clone_node(d->value.get()));
                    copy->span = d->span;
                    return copy;
                }
            case AstKind::ImportStatement:
                {
                    auto* i = static_cast<const ImportStatement*>(node);
                    auto copy = std::make_unique<ImportStatement>(
                        clone_nodes(i->modifiers), i->path);
                    copy->resolved_canonical_path = i->resolved_canonical_path;
                    copy->span = i->span;
                    return copy;
                }
            case AstKind::FunctionDefinition:
                {
                    auto* f = static_cast<const FunctionDefinition*>(node);
                    auto copy = std::make_unique<FunctionDefinition>(
                        clone_nodes(f->modifiers),
                        f->name,
                        clone_nodes(f->parameters),
                        clone_nodes(f->return_types),
                        clone_nodes(f->body),
                        f->docstring);
                    copy->span = f->span;
                    return copy;
                }
            case AstKind::StructDefinition:
                {
                    auto* s = static_cast<const StructDefinition*>(node);
                    auto copy = std::make_unique<StructDefinition>(
                        clone_nodes(s->modifiers),
                        s->name,
                        clone_nodes(s->fields));
                    copy->span = s->span;
                    return copy;
                }
            case AstKind::TypeAliasDefinition:
                {
                    auto* a = static_cast<const TypeAliasDefinition*>(node);
                    auto copy = std::make_unique<TypeAliasDefinition>(
                        clone_nodes(a->modifiers),
                        a->name,
                        clone_node(a->target_type.get()));
                    copy->span = a->span;
                    return copy;
                }
            case AstKind::ExtensionDefinition:
                {
                    auto* e = static_cast<const ExtensionDefinition*>(node);
                    auto copy = std::make_unique<ExtensionDefinition>(
                        clone_nodes(e->modifiers),
                        clone_node(e->target_type.get()));
                    copy->execution_steps = clone_nodes(e->execution_steps);
                    copy->function_definitions = clone_nodes(e->function_definitions);
                    copy->struct_definitions = clone_nodes(e->struct_definitions);
                    copy->enum_definitions = clone_nodes(e->enum_definitions);
                    copy->type_aliases = clone_nodes(e->type_aliases);
                    copy->span = e->span;
                    return copy;
                }
            case AstKind::Program:
                {
                    auto* p = static_cast<const Program*>(node);
                    auto copy = std::make_unique<Program>();
                    copy->comments = clone_nodes(p->comments);
                    copy->import_statements = clone_nodes(p->import_statements);
                    copy->directives = clone_nodes(p->directives);
                    copy->execution_steps = clone_nodes(p->execution_steps);
                    copy->function_definitions = clone_nodes(p->function_definitions);
                    copy->struct_definitions = clone_nodes(p->struct_definitions);
                    copy->enum_definitions = clone_nodes(p->enum_definitions);
                    copy->type_aliases = clone_nodes(p->type_aliases);
                    copy->extension_definitions = clone_nodes(p->extension_definitions);
                    copy->span = p->span;
                    return copy;
                }
            case AstKind::Comment:
                {
                    auto* c = static_cast<const Comment*>(node);
                    auto copy = std::make_unique<Comment>(c->text, c->span);
                    return copy;
                }
            case AstKind::Modifier:
                {
                    auto* m = static_cast<const Modifier*>(node);
                    auto copy = std::make_unique<Modifier>(
                        m->name, clone_nodes(m->arguments), m->span);
                    return copy;
                }
            case AstKind::CallArgument:
                {
                    auto* a = static_cast<const CallArgument*>(node);
                    auto copy = std::make_unique<CallArgument>(
                        a->name, clone_node(a->value.get()), a->span);
                    return copy;
                }
            case AstKind::FunctionParameter:
                {
                    auto* p = static_cast<const FunctionParameter*>(node);
                    auto copy = std::make_unique<FunctionParameter>(
                        clone_nodes(p->modifiers),
                        p->name,
                        clone_node(p->type.get()),
                        clone_node(p->default_value.get()),
                        p->span);
                    return copy;
                }
            case AstKind::StructField:
                {
                    auto* f = static_cast<const StructField*>(node);
                    auto copy = std::make_unique<StructField>(
                        clone_nodes(f->modifiers),
                        f->name,
                        clone_node(f->type.get()),
                        f->span);
                    return copy;
                }
            case AstKind::EnumCase:
                {
                    auto* c = static_cast<const EnumCase*>(node);
                    auto copy = std::make_unique<EnumCase>(
                        clone_nodes(c->modifiers),
                        c->name,
                        clone_node(c->value.get()),
                        c->span);
                    return copy;
                }
            case AstKind::DictItem:
                {
                    auto* d = static_cast<const DictItem*>(node);
                    auto copy = std::make_unique<DictItem>(
                        clone_nodes(d->modifiers),
                        d->key,
                        clone_node(d->value.get()),
                        d->span);
                    return copy;
                }
            case AstKind::SwitchCase:
                {
                    auto* s = static_cast<const SwitchCase*>(node);
                    auto copy = std::make_unique<SwitchCase>(
                        clone_nodes(s->modifiers),
                        s->identifiers,
                        clone_node(s->result.get()),
                        s->span);
                    return copy;
                }
            case AstKind::AssignmentTarget:
                {
                    auto* t = static_cast<const AssignmentTarget*>(node);
                    auto copy = std::make_unique<AssignmentTarget>(
                        clone_nodes(t->modifiers),
                        t->name,
                        clone_node(t->type.get()),
                        t->span);
                    return copy;
                }
            default:
                {
                    if (auto expr_copy = clone_expression(dynamic_cast<const Expression*>(node)))
                    {
                        return expr_copy;
                    }
                    if (auto stmt_copy = clone_statement(dynamic_cast<const Statement*>(node)))
                    {
                        return stmt_copy;
                    }
                    if (auto type_copy = clone_type_annotation(dynamic_cast<const TypeAnnotation*>(node)))
                    {
                        return type_copy;
                    }
                    return nullptr;
                }
            }
        }
    }

    inline Comment clone_node(const Comment& comment)
    {
        return Comment(comment.text, comment.span);
    }

    inline CallArgument clone_node(const CallArgument& arg)
    {
        return CallArgument(arg.name, clone_node(arg.value.get()), arg.span);
    }

    inline Modifier clone_node(const Modifier& mod)
    {
        return Modifier(mod.name, clone_nodes(mod.arguments), mod.span);
    }

    inline FunctionParameter clone_node(const FunctionParameter& param)
    {
        return FunctionParameter(
            clone_nodes(param.modifiers),
            param.name,
            clone_node(param.type.get()),
            clone_node(param.default_value.get()),
            param.span);
    }

    inline StructField clone_node(const StructField& field)
    {
        return StructField(
            clone_nodes(field.modifiers),
            field.name,
            clone_node(field.type.get()),
            field.span);
    }

    inline EnumCase clone_node(const EnumCase& item)
    {
        return EnumCase(
            clone_nodes(item.modifiers),
            item.name,
            clone_node(item.value.get()),
            item.span);
    }

    inline DictItem clone_node(const DictItem& item)
    {
        return DictItem(
            clone_nodes(item.modifiers),
            item.key,
            clone_node(item.value.get()),
            item.span);
    }

    inline SwitchCase clone_node(const SwitchCase& item)
    {
        return SwitchCase(
            clone_nodes(item.modifiers),
            item.identifiers,
            clone_node(item.result.get()),
            item.span);
    }

    inline AssignmentTarget clone_node(const AssignmentTarget& target)
    {
        return AssignmentTarget(
            clone_nodes(target.modifiers),
            target.name,
            clone_node(target.type.get()),
            target.span);
    }

    template <AstElement T>
    [[nodiscard]] std::unique_ptr<T> clone_node(const T* node)
    {
        if (!node) return nullptr;

        if constexpr (std::same_as<T, Expression>)
        {
            return detail::clone_expression(node);
        }
        else if constexpr (std::same_as<T, Statement>)
        {
            return detail::clone_statement(node);
        }
        else if constexpr (std::same_as<T, TypeAnnotation>)
        {
            return detail::clone_type_annotation(node);
        }
        else if constexpr (std::same_as<T, AstNode>)
        {
            return detail::clone_ast_node(node);
        }
        else if constexpr (ExpressionNode<T>)
        {
            auto cloned = detail::clone_expression(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
        else if constexpr (StatementNode<T>)
        {
            auto cloned = detail::clone_statement(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
        else if constexpr (TypeAnnNode<T>)
        {
            auto cloned = detail::clone_type_annotation(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
        else
        {
            auto cloned = detail::clone_ast_node(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
    }

    template <AstElement T>
    [[nodiscard]] std::unique_ptr<T> clone_node(const std::unique_ptr<T>& ptr)
    {
        return clone_node(ptr.get());
    }

    template <typename T>
    [[nodiscard]] std::vector<T> clone_nodes(const std::vector<T>& vec)
    {
        std::vector<T> result;
        result.reserve(vec.size());
        for (const auto& item : vec)
        {
            result.push_back(clone_node(item));
        }
        return result;
    }

    template <AstElement T>
    [[nodiscard]] std::vector<std::unique_ptr<T>> clone_nodes(const std::vector<std::unique_ptr<T>>& vec)
    {
        std::vector<std::unique_ptr<T>> result;
        result.reserve(vec.size());
        for (const auto& item : vec)
        {
            result.push_back(clone_node(item.get()));
        }
        return result;
    }

    template <typename T1, typename T2>
    [[nodiscard]] inline bool ast_is_clone_of(const T1& orig, const T2& clone) noexcept
    {
        return ast_equals(orig, clone) && ast_is_disjoint(orig, clone);
    }

    template <typename T>
    concept CloneableAstNode = ConcreteAstNode<T> && (
        requires(const T* node) { { clone_node(node) } -> std::same_as<std::unique_ptr<T>>; } ||
        requires(const T& node) { { clone_node(node) } -> std::same_as<T>; }
    );
}
