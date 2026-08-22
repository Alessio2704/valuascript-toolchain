#pragma once
#include "ast_core.h"
#include <optional>
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_type.h"

namespace valuascript::compiler
{
    class Directive : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Directive;
        NodeName name;
        ExprPtr value;

        explicit Directive(NodeName n, ExprPtr val)
            : AstNode(KIND), name(std::move(n)), value(std::move(val))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && value && value->is_valid();
        }
    };

    class ImportStatement : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::ImportStatement;
        std::vector<Modifier> modifiers;
        NodeName path;
        std::optional<std::string> resolved_canonical_path = std::nullopt;

        explicit ImportStatement(std::vector<Modifier> mods, NodeName p)
            : AstNode(KIND), modifiers(std::move(mods)), path(std::move(p))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return path.is_valid() && span.is_valid() && are_all_valid(modifiers);
        }
    };

    class FunctionParameter : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::FunctionParameter;
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr type;
        ExprPtr default_value = nullptr;

        FunctionParameter() : AstNode(KIND) {}
        FunctionParameter(std::vector<Modifier> mods,
                          NodeName n,
                          TypeAnnPtr t,
                          ExprPtr def = nullptr,
                          SourceSpan sp = {})
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), type(std::move(t)),
              default_value(std::move(def))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && (!type || type->is_valid()) && (!default_value || default_value->is_valid());
        }
    };

    class FunctionDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::FunctionDefinition;
        std::vector<Modifier> modifiers;
        NodeName name;
        std::vector<FunctionParameter> parameters;
        std::vector<TypeAnnPtr> return_types;
        std::vector<StmtPtr> body;
        std::optional<std::string> docstring;

        explicit FunctionDefinition(std::vector<Modifier> mods,
                                    NodeName n,
                                    std::vector<FunctionParameter> params,
                                    std::vector<TypeAnnPtr> ret_types,
                                    std::vector<StmtPtr> b,
                                    std::optional<std::string> docs = std::nullopt)
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), parameters(std::move(params)),
              return_types(std::move(ret_types)), body(std::move(b)), docstring(std::move(docs))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && are_all_valid(parameters) && are_all_valid(return_types) && are_all_valid(body);
        }
    };

    class StructField : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::StructField;
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr type;

        StructField() : AstNode(KIND) {}
        StructField(std::vector<Modifier> mods,
                    NodeName n,
                    TypeAnnPtr t,
                    SourceSpan sp = {})
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), type(std::move(t))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && (!type || type->is_valid());
        }
    };

    class StructDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::StructDefinition;
        std::vector<Modifier> modifiers;
        NodeName name;
        std::vector<StructField> fields;

        explicit StructDefinition(std::vector<Modifier> mods,
                                  NodeName n,
                                  std::vector<StructField> f)
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), fields(std::move(f))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && are_all_valid(fields);
        }
    };

    class TypeAliasDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::TypeAliasDefinition;
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr target_type;

        explicit TypeAliasDefinition(std::vector<Modifier> mods,
                                     NodeName n,
                                     TypeAnnPtr t_type)
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), target_type(std::move(t_type))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && target_type && target_type->is_valid();
        }
    };

    class ExtensionDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::ExtensionDefinition;
        std::vector<Modifier> modifiers;
        TypeAnnPtr target_type;
        std::vector<StmtPtr> execution_steps;
        std::vector<FuncDefPtr> function_definitions;
        std::vector<StructDefPtr> struct_definitions;
        std::vector<EnumDefPtr> enum_definitions;
        std::vector<TypeAliasPtr> type_aliases;

        explicit ExtensionDefinition(std::vector<Modifier> mods,
                                     TypeAnnPtr target)
            : AstNode(KIND), modifiers(std::move(mods)), target_type(std::move(target))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return target_type && target_type->is_valid() && span.is_valid() &&
                   are_all_valid(modifiers) &&
                   are_all_valid(execution_steps) &&
                   are_all_valid(function_definitions) &&
                   are_all_valid(struct_definitions) &&
                   are_all_valid(enum_definitions) &&
                   are_all_valid(type_aliases);
        }
    };

    class Program : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Program;
        std::unique_ptr<AstArena> arena = nullptr;
        std::vector<Comment> comments;
        std::vector<ImportPtr> import_statements;
        std::vector<DirectivePtr> directives;
        std::vector<StmtPtr> execution_steps;
        std::vector<FuncDefPtr> function_definitions;
        std::vector<StructDefPtr> struct_definitions;
        std::vector<EnumDefPtr> enum_definitions;
        std::vector<TypeAliasPtr> type_aliases;
        std::vector<ExtensionDefPtr> extension_definitions;

        Program() : AstNode(KIND) {}

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid() &&
                   are_all_valid(comments) &&
                   are_all_valid(import_statements) &&
                   are_all_valid(directives) &&
                   are_all_valid(execution_steps) &&
                   are_all_valid(function_definitions) &&
                   are_all_valid(struct_definitions) &&
                   are_all_valid(enum_definitions) &&
                   are_all_valid(type_aliases) &&
                   are_all_valid(extension_definitions);
        }
    };
}
