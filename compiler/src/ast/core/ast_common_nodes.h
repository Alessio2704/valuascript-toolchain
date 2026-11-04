#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "token/token.h"
#include "token/source_span.h"
#include "ast_node.h"
#include "ast_node_name.h"

namespace valuascript::compiler
{
    template <typename Container>
    [[nodiscard]] inline bool are_all_valid(const Container& items) noexcept
    {
        for (const auto& item : items)
        {
            if constexpr (requires { item->is_valid(); })
            {
                if (!item || !item->is_valid()) return false;
            }
            else if constexpr (requires { item.is_valid(); })
            {
                if (!item.is_valid()) return false;
            }
        }
        return true;
    }

    class Comment : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Comment;
        std::string text = {};

        Comment() : AstNode(KIND)
        {
        }

        Comment(std::string txt, valuascript::shared::SourceSpan sp = {})
            : AstNode(KIND), text(std::move(txt))
        {
            span = sp;
        }

        explicit Comment(const valuascript::shared::CommentToken& tok)
            : AstNode(KIND), text(tok.text)
        {
            span = tok.span;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return !text.empty() && span.is_valid();
        }
    };

    class CallArgument : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::CallArgument;
        NodeName name;
        ExprPtr value = nullptr;

        CallArgument() : AstNode(KIND)
        {
        }

        CallArgument(NodeName n, ExprPtr val = nullptr, valuascript::shared::SourceSpan sp = {})
            : AstNode(KIND), name(std::move(n)), value(std::move(val))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid() && (!value || value->is_valid());
        }
    };

    class Modifier : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Modifier;
        NodeName name;
        std::vector<CallArgument> arguments;

        Modifier() : AstNode(KIND)
        {
        }

        Modifier(NodeName n, std::vector<CallArgument> args = {}, valuascript::shared::SourceSpan sp = {})
            : AstNode(KIND), name(std::move(n)), arguments(std::move(args))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(arguments);
        }
    };
}
