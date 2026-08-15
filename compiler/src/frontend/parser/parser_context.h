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
        bool is_consuming_unexpected = false;
        bool is_parsing_expression_statement = false;
        bool is_parsing_list_element = false;
        size_t conditional_else_depth = 0;
        size_t conditional_else_closers_size = 0;
        size_t expr_depth = 0;
        size_t expr_closers_baseline = 0;
        size_t key_value_container_depth = 0;
        std::vector<size_t> parameter_list_closer_indices;
        std::vector<size_t> switch_target_closer_indices;

        explicit ParserContext(TokenCursor c);

        [[nodiscard]] bool is_active_closer(TokenType type) const;
        [[nodiscard]] bool is_in_sync_set(TokenType type) const;

        [[nodiscard]] bool looks_like_reassignment() const;

        const Token& consume_identifier(ParserErrorCode fallback_err, bool allow_top_level_keywords = true,
                                        bool check_statement_boundary = false);
        [[nodiscard]] TokenType peek_past_modifiers() const;
        [[nodiscard]] bool is_at_top_level_declaration() const;
        [[nodiscard]] bool is_at_any_declaration() const;
        [[nodiscard]] bool is_missing_closing_brace() const;
        void reject_modifiers(const std::vector<Modifier>& modifiers,
                              ParserErrorCode error_code =
                                  ParserErrorCode::ModifiersAttachedToInvalidDeclaration) const;
    };
}
