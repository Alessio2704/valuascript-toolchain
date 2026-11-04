#include "type_checker_stage.h"
#include "type_checker_error_code.h"
#include "core/valuascript_exception.h"
#include "core/error_formatter.h"
#include "token/operator_lookup.h"
#include "ast/utils/ast_cast.h"
#include "ast/walker/ast_walker.h"

namespace valuascript::compiler
{
    namespace
    {
        std::string_view get_op_symbol(TokenType type) noexcept
        {
            for (const auto& op : shared::BINARY_OPERATORS)
            {
                if (op.type == type) return op.text;
            }
            for (const auto& op : shared::UNARY_OPERATORS)
            {
                if (op.type == type) return op.text;
            }
            return "?";
        }

        class TypeInferenceWalker : public ConstAstWalker
        {
        private:
            CompilerContext& context_;
            Scope* current_scope_;
            const SymbolTableArtifact& symbol_table_;
            TypeTableArtifact& artifact_;
            TypePtr current_function_return_type_ = nullptr;
            std::string current_function_name_;

        public:
            using ConstAstWalker::enter;
            using ConstAstWalker::leave;

            TypeInferenceWalker(CompilerContext& ctx, Scope* root_scope,
                                const SymbolTableArtifact& sym_tab, TypeTableArtifact& art)
                : context_(ctx), current_scope_(root_scope), symbol_table_(sym_tab), artifact_(art)
            {
            }

            TypePtr infer_expression(const Expression* expr)
            {
                if (!expr) return make_unknown_type();

                if (auto cached = artifact_.get_type(expr))
                {
                    return cached;
                }

                TypePtr result = make_unknown_type();

                if (auto* num = ast_cast<NumberLiteral>(expr))
                {
                    result = num->value.find('.') != std::string::npos ? make_float_type() : make_int_type();
                }
                else if (ast_cast<StringLiteral>(expr))
                {
                    result = make_string_type();
                }
                else if (ast_cast<BooleanLiteral>(expr))
                {
                    result = make_bool_type();
                }
                else if (ast_cast<PercentageLiteral>(expr))
                {
                    result = make_float_type();
                }
                else if (auto* tuple_lit = ast_cast<TupleLiteral>(expr))
                {
                    std::vector<TypePtr> element_types;
                    element_types.reserve(tuple_lit->elements.size());
                    for (const auto& elem : tuple_lit->elements)
                    {
                        element_types.push_back(infer_expression(elem.get()));
                    }
                    result = make_tuple_type(std::move(element_types));
                }
                else if (auto* id = ast_cast<IdentifierAccess>(expr))
                {
                    if (auto sym = symbol_table_.get_resolved_symbol(id))
                    {
                        if (auto sym_t = artifact_.get_symbol_type(sym.get()))
                        {
                            result = sym_t;
                        }
                    }
                }
                else if (auto* dot = ast_cast<DotAccess>(expr))
                {
                    auto target_t = infer_expression(dot->target.get());
                    if (!target_t->is_unknown())
                    {
                        if (target_t->is_struct())
                        {
                            if (auto field_t = target_t->get_field_type(dot->property_name.value))
                            {
                                result = field_t;
                            }
                            else
                            {
                                ValuaScriptException ex(
                                    ValuascriptErrorCategory::Semantic,
                                    TypeCheckerErrorCode::NoSuchField,
                                    dot->span,
                                    format_error(TypeCheckerErrorCode::NoSuchField, target_t->to_string(), dot->property_name.value)
                                );
                                context_.handle_error(ex);
                            }
                        }
                        else
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::NoSuchField,
                                dot->span,
                                format_error(TypeCheckerErrorCode::NoSuchField, target_t->to_string(), dot->property_name.value)
                            );
                            context_.handle_error(ex);
                        }
                    }
                }
                else if (auto* bin = ast_cast<BinaryExpression>(expr))
                {
                    auto left_t = infer_expression(bin->left.get());
                    auto right_t = infer_expression(bin->right.get());

                    switch (bin->op)
                    {
                    case TokenType::Plus:
                    case TokenType::Minus:
                    case TokenType::Star:
                    case TokenType::Slash:
                    case TokenType::Caret:
                    case TokenType::Mod:
                        if (bin->op == TokenType::Plus && left_t->is_string() && right_t->is_string())
                        {
                            result = make_string_type();
                        }
                        else if (left_t->is_numeric() && right_t->is_numeric())
                        {
                            if (left_t->get_kind() == TypeKind::Int && right_t->get_kind() == TypeKind::Int)
                            {
                                result = make_int_type();
                            }
                            else
                            {
                                result = make_float_type();
                            }
                        }
                        else if (!left_t->is_unknown() && !right_t->is_unknown())
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                bin->span,
                                format_error(TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                             get_op_symbol(bin->op), left_t->to_string(), right_t->to_string())
                            );
                            context_.handle_error(ex);
                        }
                        break;

                    case TokenType::Equals:
                    case TokenType::NotEquals:
                        if (!left_t->is_unknown() && !right_t->is_unknown())
                        {
                            if (!left_t->is_assignable_to(*right_t) && !right_t->is_assignable_to(*left_t))
                            {
                                ValuaScriptException ex(
                                    ValuascriptErrorCategory::Semantic,
                                    TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                    bin->span,
                                    format_error(TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                                 get_op_symbol(bin->op), left_t->to_string(), right_t->to_string())
                                );
                                context_.handle_error(ex);
                            }
                        }
                        result = make_bool_type();
                        break;

                    case TokenType::Greater:
                    case TokenType::GreaterEqual:
                    case TokenType::Less:
                    case TokenType::LessEqual:
                        if (left_t->is_numeric() && right_t->is_numeric())
                        {
                            result = make_bool_type();
                        }
                        else if (!left_t->is_unknown() && !right_t->is_unknown())
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                bin->span,
                                format_error(TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                             get_op_symbol(bin->op), left_t->to_string(), right_t->to_string())
                            );
                            context_.handle_error(ex);
                            result = make_bool_type();
                        }
                        break;

                    case TokenType::And:
                    case TokenType::Or:
                        if (left_t->is_boolean() && right_t->is_boolean())
                        {
                            result = make_bool_type();
                        }
                        else if (!left_t->is_unknown() && !right_t->is_unknown())
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                bin->span,
                                format_error(TypeCheckerErrorCode::CannotApplyBinaryOperator,
                                             get_op_symbol(bin->op), left_t->to_string(), right_t->to_string())
                            );
                            context_.handle_error(ex);
                            result = make_bool_type();
                        }
                        break;

                    default:
                        break;
                    }
                }
                else if (auto* un = ast_cast<UnaryExpression>(expr))
                {
                    auto operand_t = infer_expression(un->right.get());
                    if (un->op == TokenType::Minus || un->op == TokenType::Plus)
                    {
                        if (operand_t->is_numeric())
                        {
                            result = operand_t;
                        }
                        else if (!operand_t->is_unknown())
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::CannotApplyUnaryOperator,
                                un->span,
                                format_error(TypeCheckerErrorCode::CannotApplyUnaryOperator,
                                             get_op_symbol(un->op), operand_t->to_string())
                            );
                            context_.handle_error(ex);
                        }
                    }
                    else if (un->op == TokenType::Not)
                    {
                        if (operand_t->is_boolean())
                        {
                            result = make_bool_type();
                        }
                        else if (!operand_t->is_unknown())
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::CannotApplyUnaryOperator,
                                un->span,
                                format_error(TypeCheckerErrorCode::CannotApplyUnaryOperator,
                                             get_op_symbol(un->op), operand_t->to_string())
                            );
                            context_.handle_error(ex);
                            result = make_bool_type();
                        }
                    }
                }
                else if (auto* cond = ast_cast<ConditionalExpression>(expr))
                {
                    auto cond_t = infer_expression(cond->condition.get());
                    if (!cond_t->is_boolean() && !cond_t->is_unknown())
                    {
                        ValuaScriptException ex(
                            ValuascriptErrorCategory::Semantic,
                            TypeCheckerErrorCode::ConditionNotBoolean,
                            cond->condition->span,
                            format_error(TypeCheckerErrorCode::ConditionNotBoolean, cond_t->to_string())
                        );
                        context_.handle_error(ex);
                    }
                    auto then_t = infer_expression(cond->then_branch.get());
                    auto else_t = infer_expression(cond->else_branch.get());

                    if (then_t->is_assignable_to(*else_t))
                    {
                        result = else_t;
                    }
                    else if (else_t->is_assignable_to(*then_t))
                    {
                        result = then_t;
                    }
                    else
                    {
                        result = then_t;
                    }
                }
                else if (auto* call = ast_cast<FunctionCall>(expr))
                {
                    auto callee_t = infer_expression(call->target.get());
                    if (callee_t->is_callable())
                    {
                        result = callee_t->get_return_type() ? callee_t->get_return_type() : make_void_type();

                        const auto& params = callee_t->get_param_types();
                        if (call->arguments.size() != params.size())
                        {
                            std::string callee_name = callee_t->get_name().empty() ? "anonymous" : callee_t->get_name();
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::ArgumentCountMismatch,
                                call->span,
                                format_error(TypeCheckerErrorCode::ArgumentCountMismatch,
                                             callee_name, params.size(), call->arguments.size())
                            );
                            context_.handle_error(ex);
                        }
                        else
                        {
                            for (size_t i = 0; i < call->arguments.size(); ++i)
                            {
                                const auto& arg = call->arguments[i];
                                const auto& param = params[i];

                                if (!param.label.empty() && arg.name.value != param.label)
                                {
                                    ValuaScriptException ex(
                                        ValuascriptErrorCategory::Semantic,
                                        TypeCheckerErrorCode::ArgumentLabelMismatch,
                                        arg.span,
                                        format_error(TypeCheckerErrorCode::ArgumentLabelMismatch,
                                                     param.label, arg.name.value)
                                    );
                                    context_.handle_error(ex);
                                }

                                auto arg_t = infer_expression(arg.value.get());
                                if (param.type && !arg_t->is_assignable_to(*param.type))
                                {
                                    ValuaScriptException ex(
                                        ValuascriptErrorCategory::Semantic,
                                        TypeCheckerErrorCode::TypeMismatch,
                                        arg.span,
                                        format_error(TypeCheckerErrorCode::TypeMismatch,
                                                     param.type->to_string(), arg_t->to_string())
                                    );
                                    context_.handle_error(ex);
                                }
                            }
                        }
                    }
                    else if (!callee_t->is_unknown())
                    {
                        ValuaScriptException ex(
                            ValuascriptErrorCategory::Semantic,
                            TypeCheckerErrorCode::NotCallable,
                            call->span,
                            format_error(TypeCheckerErrorCode::NotCallable, callee_t->to_string())
                        );
                        context_.handle_error(ex);
                    }
                }

                artifact_.set_type(expr, result);
                return result;
            }

            TraversalAction enter(const FunctionDefinition& fn) override
            {
                current_function_name_ = fn.name.value;
                if (auto sym = current_scope_->lookup_lexical(fn.name.value))
                {
                    if (auto fn_t = artifact_.get_symbol_type(sym.get()))
                    {
                        current_function_return_type_ = fn_t->get_return_type();
                    }
                }
                if (auto it = symbol_table_.node_scopes.find(&fn); it != symbol_table_.node_scopes.end())
                {
                    current_scope_ = it->second.get();
                }
                return TraversalAction::Continue;
            }

            void leave(const FunctionDefinition&) override
            {
                if (current_scope_->get_parent())
                {
                    current_scope_ = current_scope_->get_parent();
                }
                current_function_return_type_ = nullptr;
                current_function_name_.clear();
            }

            TraversalAction enter(const Assignment& assign) override
            {
                auto rhs_t = infer_expression(assign.value.get());

                for (const auto& target : assign.targets)
                {
                    TypePtr target_t = rhs_t;
                    if (target.type)
                    {
                        auto declared_t = TypeCheckerStage::resolve_type_annotation(target.type.get(), current_scope_);
                        if (declared_t && !declared_t->is_unknown() && !rhs_t->is_assignable_to(*declared_t))
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                TypeCheckerErrorCode::TypeMismatch,
                                assign.span,
                                format_error(TypeCheckerErrorCode::TypeMismatch,
                                             declared_t->to_string(), rhs_t->to_string())
                            );
                            context_.handle_error(ex);
                        }
                        target_t = declared_t;
                    }

                    if (auto sym = current_scope_->lookup_lexical(target.name.value))
                    {
                        artifact_.set_symbol_type(sym.get(), target_t);
                    }
                }
                return TraversalAction::Continue;
            }

            TraversalAction enter(const Reassignment& reassign) override
            {
                auto target_t = infer_expression(reassign.target.get());
                auto rhs_t = infer_expression(reassign.value.get());

                if (!target_t->is_unknown() && !rhs_t->is_unknown())
                {
                    if (!rhs_t->is_assignable_to(*target_t))
                    {
                        ValuaScriptException ex(
                            ValuascriptErrorCategory::Semantic,
                            TypeCheckerErrorCode::TypeMismatch,
                            reassign.span,
                            format_error(TypeCheckerErrorCode::TypeMismatch,
                                         target_t->to_string(), rhs_t->to_string())
                        );
                        context_.handle_error(ex);
                    }
                }
                return TraversalAction::Continue;
            }

            TraversalAction enter(const ReturnStatement& ret) override
            {
                if (current_function_return_type_)
                {
                    TypePtr actual_ret = make_void_type();
                    if (!ret.values.empty() && ret.values[0])
                    {
                        actual_ret = infer_expression(ret.values[0].get());
                    }

                    if (!actual_ret->is_assignable_to(*current_function_return_type_))
                    {
                        ValuaScriptException ex(
                            ValuascriptErrorCategory::Semantic,
                            TypeCheckerErrorCode::InvalidReturnType,
                            ret.span,
                            format_error(TypeCheckerErrorCode::InvalidReturnType,
                                         current_function_name_,
                                         current_function_return_type_->to_string(),
                                         actual_ret->to_string())
                        );
                        context_.handle_error(ex);
                    }
                }
                return TraversalAction::Continue;
            }

            TraversalAction enter(const ExpressionStatement& stmt) override
            {
                infer_expression(stmt.expr.get());
                return TraversalAction::Continue;
            }
        };
    }

    TypeCheckerStage::TypeCheckerStage()
        : CompilerStage(
            "TypeCheckerStage",
            CompilerStageArtifactCode::TypeTable,
            {CompilerStageArtifactCode::SymbolTable, CompilerStageArtifactCode::ResolvedProject}
        )
    {
    }

    TypePtr TypeCheckerStage::resolve_type_annotation(const TypeAnnotation* node, Scope* scope)
    {
        if (!node) return make_void_type();

        const auto& name = node->name.value;
        if (name == "int" || name == "Int") return make_int_type();
        if (name == "float" || name == "Float") return make_float_type();
        if (name == "bool" || name == "Bool") return make_bool_type();
        if (name == "string" || name == "String") return make_string_type();
        if (name == "void" || name == "Void") return make_void_type();
        if (name == "tensor" || name == "Tensor") return make_tensor_type();

        if (auto* tuple_ann = ast_cast<TupleTypeAnnotation>(node))
        {
            std::vector<TypePtr> elements;
            elements.reserve(tuple_ann->element_types.size());
            for (const auto& elem : tuple_ann->element_types)
            {
                elements.push_back(resolve_type_annotation(elem.get(), scope));
            }
            return make_tuple_type(std::move(elements));
        }

        if (scope)
        {
            if (auto sym = scope->lookup_lexical(name))
            {
                if (sym->get_kind() == SymbolKind::Struct)
                {
                    return make_struct_type(name);
                }
                if (sym->get_kind() == SymbolKind::Enum)
                {
                    return make_enum_type(name);
                }
            }
        }

        return make_unknown_type();
    }

    void TypeCheckerStage::register_declaration_types(CompilerContext&,
                                                      const Program& program,
                                                      Scope* module_scope,
                                                      const SymbolTableArtifact& symbol_table,
                                                      TypeTableArtifact& artifact)
    {
        // 1. Struct types
        for (const auto& struct_def : program.struct_definitions)
        {
            auto struct_t = make_struct_type(struct_def->name.value);
            for (const auto& field : struct_def->fields)
            {
                auto field_t = resolve_type_annotation(field.type.get(), module_scope);
                struct_t->set_struct_field(field.name.value, field_t);
            }
            if (auto sym = module_scope->lookup_local(struct_def->name.value))
            {
                artifact.set_symbol_type(sym.get(), struct_t);
            }
        }

        // 2. Enum types
        for (const auto& enum_def : program.enum_definitions)
        {
            auto underlying = resolve_type_annotation(enum_def->underlying_type.get(), module_scope);
            auto enum_t = make_enum_type(enum_def->name.value, underlying);
            if (auto sym = module_scope->lookup_local(enum_def->name.value))
            {
                artifact.set_symbol_type(sym.get(), enum_t);
            }
        }

        // 3. Function types
        for (const auto& func_def : program.function_definitions)
        {
            std::vector<FunctionParamType> param_types;
            param_types.reserve(func_def->parameters.size());
            for (const auto& param : func_def->parameters)
            {
                auto param_t = resolve_type_annotation(param.type.get(), module_scope);
                param_types.push_back({.label = param.name.value, .type = param_t});
            }

            TypePtr ret_t = make_void_type();
            if (!func_def->return_types.empty() && func_def->return_types[0])
            {
                ret_t = resolve_type_annotation(func_def->return_types[0].get(), module_scope);
            }

            auto fn_t = make_function_type(std::move(param_types), std::move(ret_t));
            if (auto sym = module_scope->lookup_local(func_def->name.value))
            {
                artifact.set_symbol_type(sym.get(), fn_t);
            }

            // Also register parameter symbols in the function scope
            if (auto it = symbol_table.node_scopes.find(func_def.get()); it != symbol_table.node_scopes.end())
            {
                auto fn_scope = it->second;
                for (const auto& param : func_def->parameters)
                {
                    if (auto param_sym = fn_scope->lookup_local(param.name.value))
                    {
                        auto param_t = resolve_type_annotation(param.type.get(), module_scope);
                        artifact.set_symbol_type(param_sym.get(), param_t);
                    }
                }
            }
        }

        // 4. Declared variable types
        for (const auto& step : program.execution_steps)
        {
            if (auto* assign = ast_cast<Assignment>(step.get()))
            {
                for (const auto& target : assign->targets)
                {
                    if (target.type)
                    {
                        auto declared_t = resolve_type_annotation(target.type.get(), module_scope);
                        if (auto sym = module_scope->lookup_local(target.name.value))
                        {
                            artifact.set_symbol_type(sym.get(), declared_t);
                        }
                    }
                }
            }
        }
    }

    void TypeCheckerStage::type_check_program(CompilerContext& context,
                                              const Program& program,
                                              Scope* module_scope,
                                              const SymbolTableArtifact& symbol_table,
                                              TypeTableArtifact& artifact)
    {
        TypeInferenceWalker walker(context, module_scope, symbol_table, artifact);
        walker.walk(program);
    }

    CompilerStageArtifact TypeCheckerStage::run(CompilerContext& context,
                                                const std::vector<CompilerStageArtifact>& artifacts)
    {
        const auto& project = extract_artifact_data<ResolvedProjectArtifact>(
            artifacts, CompilerStageArtifactCode::ResolvedProject
        );
        const auto& symbol_table = extract_artifact_data<SymbolTableArtifact>(
            artifacts, CompilerStageArtifactCode::SymbolTable
        );

        TypeTableArtifact artifact;

        // Process in topological order
        for (const auto& module_path : project.topological_order)
        {
            auto it = project.modules.find(module_path);
            if (it == project.modules.end() || !it->second)
            {
                continue;
            }

            const auto& program = *it->second;
            auto module_scope = symbol_table.get_module_scope(module_path);
            if (!module_scope) continue;

            register_declaration_types(context, program, module_scope.get(), symbol_table, artifact);
            type_check_program(context, program, module_scope.get(), symbol_table, artifact);
        }

        return {.code = CompilerStageArtifactCode::TypeTable, .data = artifact};
    }
}
