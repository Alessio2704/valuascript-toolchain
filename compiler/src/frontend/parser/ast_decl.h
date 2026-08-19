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
        std::string name;
        ExprPtr value;

        explicit Directive(std::string_view n, ExprPtr val)
            : AstNode(KIND), name(n), value(std::move(val))
        {
        }
    };

    class ImportStatement : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::ImportStatement;
        std::vector<Modifier> modifiers;
        std::string path;

        explicit ImportStatement(std::vector<Modifier> mods, std::string_view p)
            : AstNode(KIND), modifiers(std::move(mods)), path(p)
        {
        }
    };

    struct FunctionParameter
    {
        std::vector<Modifier> modifiers;
        std::string name;
        TypeAnnPtr type;
        ExprPtr default_value = nullptr;
    };

    class FunctionDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::FunctionDefinition;
        std::vector<Modifier> modifiers;
        std::string name;
        std::vector<FunctionParameter> parameters;
        std::vector<TypeAnnPtr> return_types;
        std::vector<StmtPtr> body;
        std::optional<std::string> docstring;

        explicit FunctionDefinition(std::vector<Modifier> mods,
                                    std::string_view n,
                                    std::vector<FunctionParameter> params,
                                    std::vector<TypeAnnPtr> ret_types,
                                    std::vector<StmtPtr> b,
                                    std::optional<std::string> docs = std::nullopt)
            : AstNode(KIND), modifiers(std::move(mods)), name(n), parameters(std::move(params)),
              return_types(std::move(ret_types)), body(std::move(b)), docstring(std::move(docs))
        {
        }
    };

    struct StructField
    {
        std::vector<Modifier> modifiers;
        std::string name;
        TypeAnnPtr type;
        SourceSpan span;
    };

    class StructDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::StructDefinition;
        std::vector<Modifier> modifiers;
        std::string name;
        std::vector<StructField> fields;

        explicit StructDefinition(std::vector<Modifier> mods,
                                  std::string_view n,
                                  std::vector<StructField> f)
            : AstNode(KIND), modifiers(std::move(mods)), name(n), fields(std::move(f))
        {
        }
    };

    class TypeAliasDefinition : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::TypeAliasDefinition;
        std::vector<Modifier> modifiers;
        std::string name;
        TypeAnnPtr target_type;

        explicit TypeAliasDefinition(std::vector<Modifier> mods,
                                     std::string_view n,
                                     TypeAnnPtr t_type)
            : AstNode(KIND), modifiers(std::move(mods)), name(n), target_type(std::move(t_type))
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
