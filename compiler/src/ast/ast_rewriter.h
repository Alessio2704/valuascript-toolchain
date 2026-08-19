#pragma once

#include <memory>
#include <vector>
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "ast_node_registry.h"

namespace valuascript::compiler
{
    class AstRewriter
    {
    public:
        virtual ~AstRewriter() = default;

        // Top-Level
        virtual std::unique_ptr<Program> rewrite_program(std::unique_ptr<Program> program)
        {
            if (!program) return nullptr;

            for (auto& imp : program->import_statements) { imp = rewrite_import_statement(std::move(imp)); }
            for (auto& dir : program->directives) { dir = rewrite_directive(std::move(dir)); }
            for (auto& st : program->struct_definitions) { st = rewrite_struct_definition(std::move(st)); }
            for (auto& en : program->enum_definitions) { en = rewrite_enum_definition(std::move(en)); }
            for (auto& ta : program->type_aliases) { ta = rewrite_type_alias_definition(std::move(ta)); }
            for (auto& fn : program->function_definitions) { fn = rewrite_function_definition(std::move(fn)); }
            for (auto& ext : program->extension_definitions) { ext = rewrite_extension_definition(std::move(ext)); }
            for (auto& step : program->execution_steps) { step = rewrite_statement(std::move(step)); }

            return program;
        }

        // Generic Category Rewriters
        virtual StmtPtr rewrite_statement(StmtPtr stmt)
        {
            if (!stmt) return nullptr;

            switch (stmt->kind)
            {
                case AstKind::Assignment:
                    return rewrite_assignment(ast_cast_unique<Assignment>(std::move(stmt)));
                case AstKind::Reassignment:
                    return rewrite_reassignment(ast_cast_unique<Reassignment>(std::move(stmt)));
                case AstKind::ExpressionStatement:
                    return rewrite_expression_statement(ast_cast_unique<ExpressionStatement>(std::move(stmt)));
                case AstKind::ReturnStatement:
                    return rewrite_return_statement(ast_cast_unique<ReturnStatement>(std::move(stmt)));
                case AstKind::EnumDefinition:
                    return rewrite_enum_definition(ast_cast_unique<EnumDefinition>(std::move(stmt)));
                default:
                    return stmt;
            }
        }

        virtual ExprPtr rewrite_expression(ExprPtr expr)
        {
            if (!expr) return nullptr;

            switch (expr->kind)
            {
                case AstKind::NumberLiteral:
                    return rewrite_number_literal(ast_cast_unique<NumberLiteral>(std::move(expr)));
                case AstKind::PercentageLiteral:
                    return rewrite_percentage_literal(ast_cast_unique<PercentageLiteral>(std::move(expr)));
                case AstKind::StringLiteral:
                    return rewrite_string_literal(ast_cast_unique<StringLiteral>(std::move(expr)));
                case AstKind::BooleanLiteral:
                    return rewrite_boolean_literal(ast_cast_unique<BooleanLiteral>(std::move(expr)));
                case AstKind::IdentifierAccess:
                    return rewrite_identifier_access(ast_cast_unique<IdentifierAccess>(std::move(expr)));
                case AstKind::SelfExpression:
                    return rewrite_self_expression(ast_cast_unique<SelfExpression>(std::move(expr)));
                case AstKind::BinaryExpression:
                    return rewrite_binary_expression(ast_cast_unique<BinaryExpression>(std::move(expr)));
                case AstKind::UnaryExpression:
                    return rewrite_unary_expression(ast_cast_unique<UnaryExpression>(std::move(expr)));
                case AstKind::GroupingExpression:
                    return rewrite_grouping_expression(ast_cast_unique<GroupingExpression>(std::move(expr)));
                case AstKind::ConditionalExpression:
                    return rewrite_conditional_expression(ast_cast_unique<ConditionalExpression>(std::move(expr)));
                case AstKind::FunctionCall:
                    return rewrite_function_call(ast_cast_unique<FunctionCall>(std::move(expr)));
                case AstKind::DictLiteral:
                    return rewrite_dict_literal(ast_cast_unique<DictLiteral>(std::move(expr)));
                case AstKind::TensorLiteral:
                    return rewrite_tensor_literal(ast_cast_unique<TensorLiteral>(std::move(expr)));
                case AstKind::TupleLiteral:
                    return rewrite_tuple_literal(ast_cast_unique<TupleLiteral>(std::move(expr)));
                case AstKind::BracketAccess:
                    return rewrite_bracket_access(ast_cast_unique<BracketAccess>(std::move(expr)));
                case AstKind::DotAccess:
                    return rewrite_dot_access(ast_cast_unique<DotAccess>(std::move(expr)));
                case AstKind::SwitchExpression:
                    return rewrite_switch_expression(ast_cast_unique<SwitchExpression>(std::move(expr)));
                default:
                    return expr;
            }
        }

        virtual TypeAnnPtr rewrite_type(TypeAnnPtr type)
        {
            if (!type) return nullptr;

            if (type->kind == AstKind::TupleTypeAnnotation)
            {
                return rewrite_tuple_type_annotation(ast_cast_unique<TupleTypeAnnotation>(std::move(type)));
            }
            return rewrite_type_annotation(std::move(type));
        }

        // Specific Declarations
        virtual ImportPtr rewrite_import_statement(ImportPtr node) { return node; }

        virtual DirectivePtr rewrite_directive(DirectivePtr node)
        {
            if (node && node->value) node->value = rewrite_expression(std::move(node->value));
            return node;
        }

        virtual FuncDefPtr rewrite_function_definition(FuncDefPtr node)
        {
            if (!node) return nullptr;
            for (auto& param : node->parameters)
            {
                if (param.type) param.type = rewrite_type(std::move(param.type));
                if (param.default_value) param.default_value = rewrite_expression(std::move(param.default_value));
            }
            for (auto& ret : node->return_types) { ret = rewrite_type(std::move(ret)); }
            for (auto& stmt : node->body) { stmt = rewrite_statement(std::move(stmt)); }
            return node;
        }

        virtual StructDefPtr rewrite_struct_definition(StructDefPtr node)
        {
            if (!node) return nullptr;
            for (auto& field : node->fields)
            {
                if (field.type) field.type = rewrite_type(std::move(field.type));
            }
            return node;
        }

        virtual EnumDefPtr rewrite_enum_definition(EnumDefPtr node)
        {
            if (!node) return nullptr;
            if (node->underlying_type) node->underlying_type = rewrite_type(std::move(node->underlying_type));
            for (auto& enum_case : node->cases)
            {
                if (enum_case.value) enum_case.value = rewrite_expression(std::move(enum_case.value));
            }
            return node;
        }

        virtual TypeAliasPtr rewrite_type_alias_definition(TypeAliasPtr node)
        {
            if (node && node->target_type) node->target_type = rewrite_type(std::move(node->target_type));
            return node;
        }

        virtual ExtensionDefPtr rewrite_extension_definition(ExtensionDefPtr node)
        {
            if (!node) return nullptr;
            if (node->target_type) node->target_type = rewrite_type(std::move(node->target_type));
            for (auto& st : node->struct_definitions) { st = rewrite_struct_definition(std::move(st)); }
            for (auto& en : node->enum_definitions) { en = rewrite_enum_definition(std::move(en)); }
            for (auto& ta : node->type_aliases) { ta = rewrite_type_alias_definition(std::move(ta)); }
            for (auto& fn : node->function_definitions) { fn = rewrite_function_definition(std::move(fn)); }
            for (auto& step : node->execution_steps) { step = rewrite_statement(std::move(step)); }
            return node;
        }

        // Specific Statements
        virtual StmtPtr rewrite_assignment(std::unique_ptr<Assignment> node)
        {
            if (!node) return nullptr;
            for (auto& target : node->targets)
            {
                if (target.type) target.type = rewrite_type(std::move(target.type));
            }
            if (node->value) node->value = rewrite_expression(std::move(node->value));
            return node;
        }

        virtual StmtPtr rewrite_reassignment(std::unique_ptr<Reassignment> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite_expression(std::move(node->target));
            if (node->value) node->value = rewrite_expression(std::move(node->value));
            return node;
        }

        virtual StmtPtr rewrite_expression_statement(std::unique_ptr<ExpressionStatement> node)
        {
            if (node && node->expr) node->expr = rewrite_expression(std::move(node->expr));
            return node;
        }

        virtual StmtPtr rewrite_return_statement(std::unique_ptr<ReturnStatement> node)
        {
            if (!node) return nullptr;
            for (auto& val : node->values) { val = rewrite_expression(std::move(val)); }
            return node;
        }

        // Specific Expressions
        virtual ExprPtr rewrite_number_literal(std::unique_ptr<NumberLiteral> node) { return node; }
        virtual ExprPtr rewrite_percentage_literal(std::unique_ptr<PercentageLiteral> node) { return node; }
        virtual ExprPtr rewrite_string_literal(std::unique_ptr<StringLiteral> node) { return node; }
        virtual ExprPtr rewrite_boolean_literal(std::unique_ptr<BooleanLiteral> node) { return node; }
        virtual ExprPtr rewrite_identifier_access(std::unique_ptr<IdentifierAccess> node) { return node; }
        virtual ExprPtr rewrite_self_expression(std::unique_ptr<SelfExpression> node) { return node; }

        virtual ExprPtr rewrite_binary_expression(std::unique_ptr<BinaryExpression> node)
        {
            if (!node) return nullptr;
            if (node->left) node->left = rewrite_expression(std::move(node->left));
            if (node->right) node->right = rewrite_expression(std::move(node->right));
            return node;
        }

        virtual ExprPtr rewrite_unary_expression(std::unique_ptr<UnaryExpression> node)
        {
            if (node && node->right) node->right = rewrite_expression(std::move(node->right));
            return node;
        }

        virtual ExprPtr rewrite_grouping_expression(std::unique_ptr<GroupingExpression> node)
        {
            if (node && node->expression) node->expression = rewrite_expression(std::move(node->expression));
            return node;
        }

        virtual ExprPtr rewrite_conditional_expression(std::unique_ptr<ConditionalExpression> node)
        {
            if (!node) return nullptr;
            if (node->condition) node->condition = rewrite_expression(std::move(node->condition));
            if (node->then_branch) node->then_branch = rewrite_expression(std::move(node->then_branch));
            if (node->else_branch) node->else_branch = rewrite_expression(std::move(node->else_branch));
            return node;
        }

        virtual ExprPtr rewrite_function_call(std::unique_ptr<FunctionCall> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite_expression(std::move(node->target));
            for (auto& arg : node->arguments)
            {
                if (arg.value) arg.value = rewrite_expression(std::move(arg.value));
            }
            return node;
        }

        virtual ExprPtr rewrite_dict_literal(std::unique_ptr<DictLiteral> node)
        {
            if (!node) return nullptr;
            for (auto& item : node->elements)
            {
                if (item.value) item.value = rewrite_expression(std::move(item.value));
            }
            return node;
        }

        virtual ExprPtr rewrite_tensor_literal(std::unique_ptr<TensorLiteral> node)
        {
            if (!node) return nullptr;
            for (auto& elem : node->elements) { elem = rewrite_expression(std::move(elem)); }
            return node;
        }

        virtual ExprPtr rewrite_tuple_literal(std::unique_ptr<TupleLiteral> node)
        {
            if (!node) return nullptr;
            for (auto& elem : node->elements) { elem = rewrite_expression(std::move(elem)); }
            return node;
        }

        virtual ExprPtr rewrite_bracket_access(std::unique_ptr<BracketAccess> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite_expression(std::move(node->target));
            if (node->index) node->index = rewrite_expression(std::move(node->index));
            return node;
        }

        virtual ExprPtr rewrite_dot_access(std::unique_ptr<DotAccess> node)
        {
            if (node && node->target) node->target = rewrite_expression(std::move(node->target));
            return node;
        }

        virtual ExprPtr rewrite_switch_expression(std::unique_ptr<SwitchExpression> node)
        {
            if (!node) return nullptr;
            if (node->target) node->target = rewrite_expression(std::move(node->target));
            for (auto& sc : node->cases)
            {
                if (sc.result) sc.result = rewrite_expression(std::move(sc.result));
            }
            if (node->default_case) node->default_case = rewrite_expression(std::move(node->default_case));
            return node;
        }

        virtual TypeAnnPtr rewrite_type_annotation(TypeAnnPtr node)
        {
            if (!node) return nullptr;
            for (auto& arg : node->generic_args) { arg = rewrite_type(std::move(arg)); }
            return node;
        }

        virtual TypeAnnPtr rewrite_tuple_type_annotation(std::unique_ptr<TupleTypeAnnotation> node)
        {
            if (!node) return nullptr;
            for (auto& elem : node->element_types) { elem = rewrite_type(std::move(elem)); }
            return node;
        }
    };

    // Static verification for AstRewriter
    static_assert(requires(AstRewriter& r, std::unique_ptr<Program> p) { { r.rewrite_program(std::move(p)) } -> std::same_as<std::unique_ptr<Program>>; });
    static_assert(requires(AstRewriter& r, StmtPtr s) { { r.rewrite_statement(std::move(s)) } -> std::same_as<StmtPtr>; });
    static_assert(requires(AstRewriter& r, ExprPtr e) { { r.rewrite_expression(std::move(e)) } -> std::same_as<ExprPtr>; });
    static_assert(requires(AstRewriter& r, TypeAnnPtr t) { { r.rewrite_type(std::move(t)) } -> std::same_as<TypeAnnPtr>; });
}
