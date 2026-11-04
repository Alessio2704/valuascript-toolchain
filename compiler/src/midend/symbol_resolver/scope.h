#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include "symbol.h"

namespace valuascript::compiler
{
    enum class ScopeKind : uint8_t
    {
        Module,
        Function,
        Struct,
        Enum,
        Block
    };

    inline std::string_view scope_kind_to_string(ScopeKind kind) noexcept
    {
        switch (kind)
        {
        case ScopeKind::Module: return "module";
        case ScopeKind::Function: return "function";
        case ScopeKind::Struct: return "struct";
        case ScopeKind::Enum: return "enum";
        case ScopeKind::Block: return "block";
        }
        return "unknown";
    }

    class Scope : public std::enable_shared_from_this<Scope>
    {
    private:
        ScopeKind kind_;
        std::string name_;
        Scope* parent_ = nullptr;
        std::unordered_map<std::string, SymbolPtr> symbols_;
        std::vector<SymbolPtr> ordered_symbols_;
        std::vector<std::shared_ptr<Scope>> children_;

    public:
        Scope(ScopeKind kind, std::string name, Scope* parent = nullptr)
            : kind_(kind), name_(std::move(name)), parent_(parent)
        {
        }

        [[nodiscard]] ScopeKind get_kind() const noexcept { return kind_; }
        [[nodiscard]] const std::string& get_name() const noexcept { return name_; }
        [[nodiscard]] Scope* get_parent() const noexcept { return parent_; }
        [[nodiscard]] const std::vector<std::shared_ptr<Scope>>& get_children() const noexcept { return children_; }
        [[nodiscard]] const std::vector<SymbolPtr>& get_symbols() const noexcept { return ordered_symbols_; }

        /// Inserts a symbol into this scope.
        /// Returns std::nullopt on success, or the conflicting SymbolPtr if already declared in this exact scope.
        std::optional<SymbolPtr> insert(SymbolPtr symbol)
        {
            if (!symbol) return std::nullopt;

            const auto& name = symbol->get_name();
            if (auto it = symbols_.find(name); it != symbols_.end())
            {
                return it->second;
            }

            symbol->set_declaring_scope(this);
            symbols_.emplace(name, symbol);
            ordered_symbols_.push_back(symbol);
            return std::nullopt;
        }

        /// Looks up a symbol strictly in this scope (non-recursive).
        [[nodiscard]] SymbolPtr lookup_local(std::string_view name) const
        {
            auto it = symbols_.find(std::string(name));
            if (it != symbols_.end())
            {
                return it->second;
            }
            return nullptr;
        }

        /// Looks up a symbol lexically, searching this scope then ascending parent scopes.
        [[nodiscard]] SymbolPtr lookup_lexical(std::string_view name) const
        {
            if (auto sym = lookup_local(name))
            {
                return sym;
            }
            if (parent_)
            {
                return parent_->lookup_lexical(name);
            }
            return nullptr;
        }

        /// Creates and attaches a child scope.
        std::shared_ptr<Scope> create_child(ScopeKind kind, std::string name = "")
        {
            auto child = std::make_shared<Scope>(kind, std::move(name), this);
            children_.push_back(child);
            return child;
        }

        [[nodiscard]] bool is_root() const noexcept
        {
            return parent_ == nullptr;
        }

        [[nodiscard]] size_t symbol_count() const noexcept
        {
            return symbols_.size();
        }
    };

    using ScopePtr = std::shared_ptr<Scope>;
}
