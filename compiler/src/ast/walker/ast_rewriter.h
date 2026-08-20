#pragma once

#include <memory>
#include <vector>
#include <concepts>
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    class AstRewriter
    {
    public:
        virtual ~AstRewriter() = default;

        virtual std::unique_ptr<Program> rewrite(std::unique_ptr<Program> program)
        {
            if (!program) return nullptr;

            for (auto& comm : program->comments) { comm = rewrite(std::move(comm)); }
            for (auto& imp : program->import_statements) { imp = rewrite(std::move(imp)); }
            for (auto& dir : program->directives) { dir = rewrite(std::move(dir)); }
            for (auto& st : program->struct_definitions) { st = rewrite(std::move(st)); }
            for (auto& en : program->enum_definitions) { en = rewrite(std::move(en)); }
            for (auto& ta : program->type_aliases) { ta = rewrite(std::move(ta)); }
            for (auto& fn : program->function_definitions) { fn = rewrite(std::move(fn)); }
            for (auto& ext : program->extension_definitions) { ext = rewrite(std::move(ext)); }
            for (auto& step : program->execution_steps) { step = rewrite(std::move(step)); }

            return program;
        }

        virtual StmtPtr rewrite(StmtPtr stmt)
        {
            if (!stmt) return nullptr;

            switch (stmt->kind)
            {
                case AstKind::Assignment:
                    return rewrite(ast_cast_unique<Assignment>(std::move(stmt)));
                case AstKind::Reassignment:
                    return rewrite(ast_cast_unique<Reassignment>(std::move(stmt)));
                case AstKind::ExpressionStatement:
                    return rewrite(ast_cast_unique<ExpressionStatement>(std::move(stmt)));
                case AstKind::ReturnStatement:
                    return rewrite(ast_cast_unique<ReturnStatement>(std::move(stmt)));
                case AstKind::EnumDefinition:
                    return rewrite(ast_cast_unique<EnumDefinition>(std::move(stmt)));
                default:
                    return stmt;
            }
        }

        virtual ExprPtr rewrite(ExprPtr expr)
        {
            if (!expr) return nullptr;

            switch (expr->kind)
            {
                case AstKind::NumberLiteral:
                    return rewrite(ast_cast_unique<NumberLiteral>(std::move(expr)));
                case AstKind::PercentageLiteral:
                    return rewrite(ast_cast_unique<PercentageLiteral>(std::move(expr)));
                case AstKind::StringLiteral:
                    return rewrite(ast_cast_unique<StringLiteral>(std::move(expr)));
                case AstKind::BooleanLiteral:
                    return rewrite(ast_cast_unique<BooleanLiteral>(std::move(expr)));
                case AstKind::IdentifierAccess:
                    return rewrite(ast_cast_unique<IdentifierAccess>(std::move(expr)));
                case AstKind::SelfExpression:
                    return rewrite(ast_cast_unique<SelfExpression>(std::move(expr)));
                case AstKind::BinaryExpression:
                    return rewrite(ast_cast_unique<BinaryExpression>(std::move(expr)));
                case AstKind::UnaryExpression:
                    return rewrite(ast_cast_unique<UnaryExpression>(std::move(expr)));
                case AstKind::GroupingExpression:
                    return rewrite(ast_cast_unique<GroupingExpression>(std::move(expr)));
                case AstKind::ConditionalExpression:
                    return rewrite(ast_cast_unique<ConditionalExpression>(std::move(expr)));
                case AstKind::FunctionCall:
                    return rewrite(ast_cast_unique<FunctionCall>(std::move(expr)));
                case AstKind::DictLiteral:
                    return rewrite(ast_cast_unique<DictLiteral>(std::move(expr)));
                case AstKind::TensorLiteral:
                    return rewrite(ast_cast_unique<TensorLiteral>(std::move(expr)));
                case AstKind::TupleLiteral:
                    return rewrite(ast_cast_unique<TupleLiteral>(std::move(expr)));
                case AstKind::BracketAccess:
                    return rewrite(ast_cast_unique<BracketAccess>(std::move(expr)));
                case AstKind::DotAccess:
                    return rewrite(ast_cast_unique<DotAccess>(std::move(expr)));
                case AstKind::SwitchExpression:
                    return rewrite(ast_cast_unique<SwitchExpression>(std::move(expr)));
                default:
                    return expr;
            }
        }

        virtual TypeAnnPtr rewrite(TypeAnnPtr type)
        {
            if (!type) return nullptr;

            if (type->kind == AstKind::TupleTypeAnnotation)
            {
                return rewrite(ast_cast_unique<TupleTypeAnnotation>(std::move(type)));
            }
            for (auto& arg : type->generic_args) { arg = rewrite(std::move(arg)); }
            return type;
        }

        virtual ImportPtr rewrite(ImportPtr node) { return node; }

        virtual DirectivePtr rewrite(DirectivePtr node)
        {
            if (node && node->value) node->value = rewrite(std::move(node->value));
            return node;
        }

        virtual FuncDefPtr rewrite(FuncDefPtr node)
        {
            if (!node) return nullptr;
            for (auto& mod : node->modifiers) { mod = rewrite(std::move(mod)); }
            for (auto& param : node->parameters) { param = rewrite(std::move(param)); }
            for (auto& ret : node->return_types) { ret = rewrite(std::move(ret)); }
            for (auto& stmt : node->body) { stmt = rewrite(std::move(stmt)); }
            return node;
        }

        virtual StructDefPtr rewrite(StructDefPtr node)
        {
            if (!node) return nullptr;
            for (auto& mod : node->modifiers) { mod = rewrite(std::move(mod)); }
            for (auto& field : node->fields) { field = rewrite(std::move(field)); }
            return node;
        }

        virtual EnumDefPtr rewrite(EnumDefPtr node)
        {
            if (!node) return nullptr;
            for (auto& mod : node->modifiers) { mod = rewrite(std::move(mod)); }
            if (node->underlying_type) node->underlying_type = rewrite(std::move(node->underlying_type));
            for (auto& enum_case : node->cases) { enum_case = rewrite(std::move(enum_case)); }
            return node;
        }

        virtual TypeAliasPtr rewrite(TypeAliasPtr node)
        {
            if (!node) return nullptr;
            for (auto& mod : node->modifiers) { mod = rewrite(std::move(mod)); }
            if (node->target_type) node->target_type = rewrite(std::move(node->target_type));
            return node;
        }

        virtual ExtensionDefPtr rewrite(ExtensionDefPtr node)
        {
            if (!node) return nullptr;
            for (auto& mod : node->modifiers) { mod = rewrite(std::move(mod)); }
            if (node->target_type) node->target_type = rewrite(std::move(node->target_type));
            for (auto& st : node->struct_definitions) { st = rewrite(std::move(st)); }
            for (auto& en : node->enum_definitions) { en = rewrite(std::move(en)); }
            for (auto& ta : node->type_aliases) { ta = rewrite(std::move(ta)); }
            for (auto& fn : node->function_definitions) { fn = rewrite(std::move(fn)); }
            for (auto& step : node->execution_steps) { step = rewrite(std::move(step)); }
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<Assignment> node)
        {
            if (!node) return nullptr;
            for (auto& target : node->targets) { target = rewrite(std::move(target)); }
            if (node->value) node->value = rewrite(std::move(node->value));
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<Reassignment> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite(std::move(node->target));
            if (node->value) node->value = rewrite(std::move(node->value));
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<ExpressionStatement> node)
        {
            if (node && node->expr) node->expr = rewrite(std::move(node->expr));
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<ReturnStatement> node)
        {
            if (!node) return nullptr;
            for (auto& val : node->values) { val = rewrite(std::move(val)); }
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<NumberLiteral> node) { return node; }
        virtual ExprPtr rewrite(std::unique_ptr<PercentageLiteral> node) { return node; }
        virtual ExprPtr rewrite(std::unique_ptr<StringLiteral> node) { return node; }
        virtual ExprPtr rewrite(std::unique_ptr<BooleanLiteral> node) { return node; }
        virtual ExprPtr rewrite(std::unique_ptr<IdentifierAccess> node) { return node; }
        virtual ExprPtr rewrite(std::unique_ptr<SelfExpression> node) { return node; }

        virtual ExprPtr rewrite(std::unique_ptr<BinaryExpression> node)
        {
            if (!node) return nullptr;
            if (node->left) node->left = rewrite(std::move(node->left));
            if (node->right) node->right = rewrite(std::move(node->right));
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<UnaryExpression> node)
        {
            if (node && node->right) node->right = rewrite(std::move(node->right));
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<GroupingExpression> node)
        {
            if (node && node->expression) node->expression = rewrite(std::move(node->expression));
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<ConditionalExpression> node)
        {
            if (!node) return nullptr;
            if (node->condition) node->condition = rewrite(std::move(node->condition));
            if (node->then_branch) node->then_branch = rewrite(std::move(node->then_branch));
            if (node->else_branch) node->else_branch = rewrite(std::move(node->else_branch));
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<FunctionCall> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite(std::move(node->target));
            for (auto& arg : node->arguments) { arg = rewrite(std::move(arg)); }
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<DictLiteral> node)
        {
            if (!node) return nullptr;
            for (auto& item : node->elements) { item = rewrite(std::move(item)); }
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<TensorLiteral> node)
        {
            if (!node) return nullptr;
            for (auto& elem : node->elements) { elem = rewrite(std::move(elem)); }
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<TupleLiteral> node)
        {
            if (!node) return nullptr;
            for (auto& elem : node->elements) { elem = rewrite(std::move(elem)); }
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<BracketAccess> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite(std::move(node->target));
            if (node->index) node->index = rewrite(std::move(node->index));
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<DotAccess> node)
        {
            if (node && node->target) node->target = rewrite(std::move(node->target));
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<SwitchExpression> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite(std::move(node->target));
            for (auto& sc : node->cases) { sc = rewrite(std::move(sc)); }
            if (node->default_case) node->default_case = rewrite(std::move(node->default_case));
            return node;
        }

        virtual TypeAnnPtr rewrite(std::unique_ptr<TupleTypeAnnotation> node)
        {
            if (!node) return nullptr;
            for (auto& elem : node->element_types) { elem = rewrite(std::move(elem)); }
            return node;
        }

        virtual FunctionParameter rewrite(FunctionParameter node)
        {
            for (auto& mod : node.modifiers) { mod = rewrite(std::move(mod)); }
            if (node.type) node.type = rewrite(std::move(node.type));
            if (node.default_value) node.default_value = rewrite(std::move(node.default_value));
            return node;
        }

        virtual StructField rewrite(StructField node)
        {
            for (auto& mod : node.modifiers) { mod = rewrite(std::move(mod)); }
            if (node.type) node.type = rewrite(std::move(node.type));
            return node;
        }

        virtual EnumCase rewrite(EnumCase node)
        {
            for (auto& mod : node.modifiers) { mod = rewrite(std::move(mod)); }
            if (node.value) node.value = rewrite(std::move(node.value));
            return node;
        }

        virtual SwitchCase rewrite(SwitchCase node)
        {
            if (node.result) node.result = rewrite(std::move(node.result));
            return node;
        }

        virtual AssignmentTarget rewrite(AssignmentTarget node)
        {
            if (node.type) node.type = rewrite(std::move(node.type));
            return node;
        }

        virtual Modifier rewrite(Modifier node)
        {
            for (auto& arg : node.arguments) { arg = rewrite(std::move(arg)); }
            return node;
        }

        virtual CallArgument rewrite(CallArgument node)
        {
            if (node.value) node.value = rewrite(std::move(node.value));
            return node;
        }

        virtual DictItem rewrite(DictItem node)
        {
            if (node.value) node.value = rewrite(std::move(node.value));
            return node;
        }

        virtual Comment rewrite(Comment node) { return node; }
    };
}
