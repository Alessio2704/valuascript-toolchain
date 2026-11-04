#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "scope.h"

namespace valuascript::compiler
{
    struct SymbolTableArtifact
    {
        std::unordered_map<std::string, ScopePtr> module_scopes = {};
        std::unordered_map<const AstNode*, SymbolPtr> resolved_symbols = {};
        std::unordered_map<const AstNode*, ScopePtr> node_scopes = {};

        [[nodiscard]] ScopePtr get_module_scope(const std::string& file_path) const
        {
            auto it = module_scopes.find(file_path);
            if (it != module_scopes.end())
            {
                return it->second;
            }
            return nullptr;
        }

        [[nodiscard]] SymbolPtr get_resolved_symbol(const AstNode* node) const
        {
            if (!node) return nullptr;
            auto it = resolved_symbols.find(node);
            if (it != resolved_symbols.end())
            {
                return it->second;
            }
            return nullptr;
        }

        [[nodiscard]] ScopePtr get_scope_at(const AstNode* node) const
        {
            if (!node) return nullptr;
            auto it = node_scopes.find(node);
            if (it != node_scopes.end())
            {
                return it->second;
            }
            return nullptr;
        }
    };
}
