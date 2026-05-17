#pragma once
#include <vector>
#include <functional>
#include "token_cursor.h"
#include "token_traits.h"
#include "ast.h"

namespace valuascript::compiler
{
    class ParserContext
    {
    public:
        TokenCursor cursor;
        std::vector<TokenType> active_closers;
        std::vector<TokenType> sync_set;
        std::function<void()> on_unexpected_statement;

        explicit ParserContext(TokenCursor c);

        bool is_active_closer(TokenType type) const;
        bool is_in_sync_set(TokenType type) const;

        const Token& consume_identifier(ValuascriptErrorCode fallback_err, bool allow_top_level_keywords = true,
                                        bool check_statement_boundary = false);
        TokenType peek_past_modifiers() const;
        bool is_at_top_level_declaration() const;
        bool is_at_any_declaration() const;
        bool is_missing_closing_brace() const;
        void reject_modifiers(const std::vector<Modifier>& modifiers,
                              ValuascriptErrorCode error_code =
                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration) const;
    };
}
