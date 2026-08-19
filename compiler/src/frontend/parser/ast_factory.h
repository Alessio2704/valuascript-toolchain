#pragma once
#include <memory>
#include <utility>
#include "token_cursor.h"
#include "ast/ast.h"

namespace valuascript::compiler
{
    class AstFactory
    {
    public:
        template <typename T, typename... Args>
        static std::unique_ptr<T> make_node(TokenCursor& cursor, const Token& start_token, Args&&... args)
        {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            node->span = cursor.make_span(start_token, cursor.previous());
            return node;
        }

        template <typename T, typename... Args>
        static std::unique_ptr<T> make_node_with_span(const SourceSpan& span, Args&&... args)
        {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            node->span = span;
            return node;
        }
    };
}
