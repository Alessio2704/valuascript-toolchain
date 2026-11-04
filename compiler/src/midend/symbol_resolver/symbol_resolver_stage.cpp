#include "symbol_resolver_stage.h"
#include "symbol_resolver_error_code.h"
#include "core/valuascript_exception.h"
#include "core/error_formatter.h"
#include "ast/utils/ast_cast.h"
#include "ast/walker/ast_walker.h"

namespace valuascript::compiler
{
    namespace
    {
        class BodyResolutionWalker : public ConstAstWalker
        {
        private:
            CompilerContext& context_;
            ScopePtr current_scope_;
            SymbolTableArtifact& artifact_;

        public:
            using ConstAstWalker::enter;
            using ConstAstWalker::leave;

            BodyResolutionWalker(CompilerContext& ctx, ScopePtr root_scope, SymbolTableArtifact& art)
                : context_(ctx), current_scope_(root_scope), artifact_(art)
            {
            }

            TraversalAction enter(const FunctionDefinition& fn) override
            {
                if (auto it = artifact_.node_scopes.find(&fn); it != artifact_.node_scopes.end())
                {
                    current_scope_ = it->second;
                }
                return TraversalAction::Continue;
            }

            void leave(const FunctionDefinition&) override
            {
                if (current_scope_->get_parent())
                {
                    current_scope_ = current_scope_->get_parent()->shared_from_this();
                }
            }

            TraversalAction enter(const Assignment&) override
            {
                return TraversalAction::Continue;
            }

            void leave(const Assignment& assign) override
            {
                if (current_scope_->get_kind() != ScopeKind::Module)
                {
                    for (const auto& target : assign.targets)
                    {
                        auto sym = std::make_shared<Symbol>(
                            target.name.value,
                            SymbolKind::Variable,
                            target.span,
                            &assign
                        );
                        if (auto conflict = current_scope_->insert(sym))
                        {
                            ValuaScriptException ex(
                                ValuascriptErrorCategory::Semantic,
                                SymbolResolverErrorCode::RedeclarationInSameScope,
                                target.span,
                                format_error(SymbolResolverErrorCode::RedeclarationInSameScope, target.name.value)
                            );
                            context_.handle_error(ex);
                        }
                    }
                }
            }

            TraversalAction enter(const IdentifierAccess& id) override
            {
                if (auto sym = current_scope_->lookup_lexical(id.name.value))
                {
                    artifact_.resolved_symbols[&id] = sym;
                }
                else
                {
                    ValuaScriptException ex(
                        ValuascriptErrorCategory::Semantic,
                        SymbolResolverErrorCode::UndeclaredIdentifier,
                        id.span,
                        format_error(SymbolResolverErrorCode::UndeclaredIdentifier, id.name.value)
                    );
                    context_.handle_error(ex);
                }
                return TraversalAction::Continue;
            }
        };
    }

    SymbolResolverStage::SymbolResolverStage()
        : CompilerStage(
            "SymbolResolverStage",
            CompilerStageArtifactCode::SymbolTable,
            {CompilerStageArtifactCode::ResolvedProject}
        )
    {
    }

    void SymbolResolverStage::collect_declarations(CompilerContext& context,
                                                  const std::string&,
                                                  const Program& program,
                                                  ScopePtr module_scope,
                                                  SymbolTableArtifact& artifact)
    {
        // 1. Process imports: bring exported top-level symbols from imported modules into module_scope
        for (const auto& import_stmt : program.import_statements)
        {
            if (import_stmt->resolved_canonical_path.has_value())
            {
                const std::string& dep_path = *import_stmt->resolved_canonical_path;
                if (auto dep_scope = artifact.get_module_scope(dep_path))
                {
                    for (const auto& dep_sym : dep_scope->get_symbols())
                    {
                        module_scope->insert(dep_sym);
                    }
                }
            }
        }

        // 2. Struct definitions
        for (const auto& struct_def : program.struct_definitions)
        {
            auto sym = std::make_shared<Symbol>(
                struct_def->name.value,
                SymbolKind::Struct,
                struct_def->span,
                struct_def.get()
            );
            if (auto conflict = module_scope->insert(sym))
            {
                ValuaScriptException ex(
                    ValuascriptErrorCategory::Semantic,
                    SymbolResolverErrorCode::RedeclarationInSameScope,
                    struct_def->span,
                    format_error(SymbolResolverErrorCode::RedeclarationInSameScope, struct_def->name.value)
                );
                context.handle_error(ex);
            }

            auto struct_scope = module_scope->create_child(ScopeKind::Struct, struct_def->name.value);
            artifact.node_scopes[struct_def.get()] = struct_scope;

            for (const auto& field : struct_def->fields)
            {
                auto field_sym = std::make_shared<Symbol>(
                    field.name.value,
                    SymbolKind::StructField,
                    field.span,
                    &field
                );
                if (auto field_conflict = struct_scope->insert(field_sym))
                {
                    ValuaScriptException ex(
                        ValuascriptErrorCategory::Semantic,
                        SymbolResolverErrorCode::RedeclarationInSameScope,
                        field.span,
                        format_error(SymbolResolverErrorCode::RedeclarationInSameScope, field.name.value)
                    );
                    context.handle_error(ex);
                }
            }
        }

        // 3. Enum definitions
        for (const auto& enum_def : program.enum_definitions)
        {
            auto sym = std::make_shared<Symbol>(
                enum_def->name.value,
                SymbolKind::Enum,
                enum_def->span,
                enum_def.get()
            );
            if (auto conflict = module_scope->insert(sym))
            {
                ValuaScriptException ex(
                    ValuascriptErrorCategory::Semantic,
                    SymbolResolverErrorCode::RedeclarationInSameScope,
                    enum_def->span,
                    format_error(SymbolResolverErrorCode::RedeclarationInSameScope, enum_def->name.value)
                );
                context.handle_error(ex);
            }

            auto enum_scope = module_scope->create_child(ScopeKind::Enum, enum_def->name.value);
            artifact.node_scopes[enum_def.get()] = enum_scope;

            for (const auto& enum_case : enum_def->cases)
            {
                auto case_sym = std::make_shared<Symbol>(
                    enum_case.name.value,
                    SymbolKind::EnumCase,
                    enum_case.span,
                    &enum_case
                );
                if (auto case_conflict = enum_scope->insert(case_sym))
                {
                    ValuaScriptException ex(
                        ValuascriptErrorCategory::Semantic,
                        SymbolResolverErrorCode::RedeclarationInSameScope,
                        enum_case.span,
                        format_error(SymbolResolverErrorCode::RedeclarationInSameScope, enum_case.name.value)
                    );
                    context.handle_error(ex);
                }
            }
        }

        // 4. Type aliases
        for (const auto& alias_def : program.type_aliases)
        {
            auto sym = std::make_shared<Symbol>(
                alias_def->name.value,
                SymbolKind::TypeAlias,
                alias_def->span,
                alias_def.get()
            );
            if (auto conflict = module_scope->insert(sym))
            {
                ValuaScriptException ex(
                    ValuascriptErrorCategory::Semantic,
                    SymbolResolverErrorCode::RedeclarationInSameScope,
                    alias_def->span,
                    format_error(SymbolResolverErrorCode::RedeclarationInSameScope, alias_def->name.value)
                );
                context.handle_error(ex);
            }
        }

        // 5. Function definitions
        for (const auto& func_def : program.function_definitions)
        {
            auto sym = std::make_shared<Symbol>(
                func_def->name.value,
                SymbolKind::Function,
                func_def->span,
                func_def.get()
            );
            if (auto conflict = module_scope->insert(sym))
            {
                ValuaScriptException ex(
                    ValuascriptErrorCategory::Semantic,
                    SymbolResolverErrorCode::RedeclarationInSameScope,
                    func_def->span,
                    format_error(SymbolResolverErrorCode::RedeclarationInSameScope, func_def->name.value)
                );
                context.handle_error(ex);
            }

            auto fn_scope = module_scope->create_child(ScopeKind::Function, func_def->name.value);
            artifact.node_scopes[func_def.get()] = fn_scope;

            for (const auto& param : func_def->parameters)
            {
                auto param_sym = std::make_shared<Symbol>(
                    param.name.value,
                    SymbolKind::Parameter,
                    param.span,
                    &param
                );
                if (auto param_conflict = fn_scope->insert(param_sym))
                {
                    ValuaScriptException ex(
                        ValuascriptErrorCategory::Semantic,
                        SymbolResolverErrorCode::RedeclarationInSameScope,
                        param.span,
                        format_error(SymbolResolverErrorCode::RedeclarationInSameScope, param.name.value)
                    );
                    context.handle_error(ex);
                }
            }
        }

        // 6. Top-level assignments (module-level variables)
        for (const auto& step : program.execution_steps)
        {
            if (auto* assign = ast_cast<Assignment>(step.get()))
            {
                for (const auto& target : assign->targets)
                {
                    auto sym = std::make_shared<Symbol>(
                        target.name.value,
                        SymbolKind::Variable,
                        target.span,
                        assign
                    );
                    if (auto conflict = module_scope->insert(sym))
                    {
                        ValuaScriptException ex(
                            ValuascriptErrorCategory::Semantic,
                            SymbolResolverErrorCode::RedeclarationInSameScope,
                            target.span,
                            format_error(SymbolResolverErrorCode::RedeclarationInSameScope, target.name.value)
                        );
                        context.handle_error(ex);
                    }
                }
            }
        }
    }

    void SymbolResolverStage::resolve_bodies(CompilerContext& context,
                                            const std::string&,
                                            const Program& program,
                                            ScopePtr module_scope,
                                            SymbolTableArtifact& artifact)
    {
        BodyResolutionWalker walker(context, module_scope, artifact);
        walker.walk(program);
    }

    CompilerStageArtifact SymbolResolverStage::run(CompilerContext& context,
                                                   const std::vector<CompilerStageArtifact>& artifacts)
    {
        const auto& project = extract_artifact_data<ResolvedProjectArtifact>(
            artifacts, CompilerStageArtifactCode::ResolvedProject
        );

        SymbolTableArtifact artifact;

        for (const auto& module_path : project.topological_order)
        {
            auto it = project.modules.find(module_path);
            if (it == project.modules.end() || !it->second)
            {
                continue;
            }

            const auto& program = *it->second;
            auto module_scope = std::make_shared<Scope>(ScopeKind::Module, module_path);
            artifact.module_scopes[module_path] = module_scope;
            artifact.node_scopes[&program] = module_scope;

            collect_declarations(context, module_path, program, module_scope, artifact);
            resolve_bodies(context, module_path, program, module_scope, artifact);
        }

        return {.code = CompilerStageArtifactCode::SymbolTable, .data = artifact};
    }
}
