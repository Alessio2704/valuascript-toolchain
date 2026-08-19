#pragma once
#include "ast_core.h"
#include "optional"
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
    };

    struct FunctionParameter
    {
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr type;
        ExprPtr default_value = nullptr;
        SourceSpan span = {};
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
    };

    struct StructField
    {
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr type;
        SourceSpan span = {};
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
    };

    class Program : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Program;
        std::unique_ptr<AstArena> arena = nullptr;
        std::vector<CommentToken> comments;
        std::vector<ImportPtr> import_statements;
        std::vector<DirectivePtr> directives;
        std::vector<StmtPtr> execution_steps;
        std::vector<FuncDefPtr> function_definitions;
        std::vector<StructDefPtr> struct_definitions;
        std::vector<EnumDefPtr> enum_definitions;
        std::vector<TypeAliasPtr> type_aliases;
        std::vector<ExtensionDefPtr> extension_definitions;

        Program() : AstNode(KIND) {}
    };
}
