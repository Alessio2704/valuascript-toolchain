#pragma once

#include <string>
#include <string_view>
#include <memory>
#include "token/source_span.h"
#include "ast/core/ast_node.h"

namespace valuascript::compiler
{
    class Scope;

    enum class SymbolKind : uint8_t
    {
        Variable,
        Parameter,
        Function,
        Struct,
        StructField,
        Enum,
        EnumCase,
        TypeAlias
    };

    inline std::string_view symbol_kind_to_string(SymbolKind kind) noexcept
    {
        switch (kind)
        {
        case SymbolKind::Variable: return "variable";
        case SymbolKind::Parameter: return "parameter";
        case SymbolKind::Function: return "function";
        case SymbolKind::Struct: return "struct";
        case SymbolKind::StructField: return "field";
        case SymbolKind::Enum: return "enum";
        case SymbolKind::EnumCase: return "enum case";
        case SymbolKind::TypeAlias: return "type alias";
        }
        return "unknown";
    }

    class Symbol
    {
    private:
        std::string name_;
        SymbolKind kind_;
        SourceSpan span_;
        const AstNode* ast_node_ = nullptr;
        Scope* declaring_scope_ = nullptr;

    public:
        Symbol(std::string name, SymbolKind kind, SourceSpan span, const AstNode* ast_node = nullptr)
            : name_(std::move(name)), kind_(kind), span_(std::move(span)), ast_node_(ast_node)
        {
        }

        [[nodiscard]] const std::string& get_name() const noexcept { return name_; }
        [[nodiscard]] SymbolKind get_kind() const noexcept { return kind_; }
        [[nodiscard]] const SourceSpan& get_span() const noexcept { return span_; }
        [[nodiscard]] const AstNode* get_ast_node() const noexcept { return ast_node_; }
        [[nodiscard]] Scope* get_declaring_scope() const noexcept { return declaring_scope_; }

        void set_declaring_scope(Scope* scope) noexcept { declaring_scope_ = scope; }

        [[nodiscard]] bool is_type() const noexcept
        {
            return kind_ == SymbolKind::Struct ||
                   kind_ == SymbolKind::Enum ||
                   kind_ == SymbolKind::TypeAlias;
        }

        [[nodiscard]] bool is_callable() const noexcept
        {
            return kind_ == SymbolKind::Function;
        }

        [[nodiscard]] bool is_value() const noexcept
        {
            return kind_ == SymbolKind::Variable ||
                   kind_ == SymbolKind::Parameter ||
                   kind_ == SymbolKind::EnumCase ||
                   kind_ == SymbolKind::StructField;
        }
    };

    using SymbolPtr = std::shared_ptr<Symbol>;
}
