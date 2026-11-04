#pragma once

#include <unordered_map>
#include <memory>
#include "type.h"
#include "ast/core/ast_node.h"
#include "midend/symbol_resolver/symbol.h"

namespace valuascript::compiler
{
    struct TypeTableArtifact
    {
        std::unordered_map<const AstNode*, TypePtr> expression_types = {};
        std::unordered_map<const Symbol*, TypePtr> symbol_types = {};

        [[nodiscard]] TypePtr get_type(const AstNode* node) const
        {
            if (!node) return nullptr;
            auto it = expression_types.find(node);
            if (it != expression_types.end())
            {
                return it->second;
            }
            return nullptr;
        }

        [[nodiscard]] TypePtr get_symbol_type(const Symbol* sym) const
        {
            if (!sym) return nullptr;
            auto it = symbol_types.find(sym);
            if (it != symbol_types.end())
            {
                return it->second;
            }
            return nullptr;
        }

        void set_type(const AstNode* node, TypePtr type)
        {
            if (node && type)
            {
                expression_types[node] = std::move(type);
            }
        }

        void set_symbol_type(const Symbol* sym, TypePtr type)
        {
            if (sym && type)
            {
                symbol_types[sym] = std::move(type);
            }
        }
    };
}
