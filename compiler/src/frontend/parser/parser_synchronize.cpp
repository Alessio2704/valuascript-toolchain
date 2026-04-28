#include "parser.h"
#include <algorithm>

namespace valuascript::compiler
{
    bool Parser::is_active_closer(TokenType type) const
    {
        return std::find(active_closers_.begin(), active_closers_.end(), type) != active_closers_.end();
    }

    bool Parser::is_in_sync_set(TokenType type) const
    {
        return std::find(sync_set_.begin(), sync_set_.end(), type) != sync_set_.end();
    }

    void Parser::recover(const SyncPredicate& stop_condition)
    {
        int depth = 0;
        while (!cursor_.is_at_end())
        {
            TokenType t = cursor_.peek().type;

            if (stop_condition(t, depth)) return;

            if (TokenTraits::is_grouping_opener(t))
            {
                depth++;
            }
            else if (TokenTraits::is_grouping_closer(t))
            {
                depth--;
            }

            cursor_.advance();
            if (depth < 0) depth = 0;
        }
    }

    void Parser::synchronize_with(const RecoveryConfig& config)
    {
        recover([&](TokenType type, int depth)
        {
            if (config.skip_nested_groupings_during_recovery && depth > 0) return false;

            for (TokenType t : config.stop_tokens)
            {
                if (type == t) return true;
            }

            const Token& tok = cursor_.peek();
            const TokenType next = cursor_.peek(1).type;

            if (config.force_stop_at_statement_boundary_ignoring_dangling_op)
            {
                if (TokenTraits::is_statement_start(tok, next) ||
                    (tok.line > cursor_.previous().line && TokenTraits::is_expression_statement_start(tok, next)))
                {
                    return true;
                }
            }

            if (config.stop_at_statement_boundary_respecting_dangling_op)
            {
                bool is_boundary = TokenTraits::is_statement_start(tok, next) ||
                    TokenTraits::is_newline_statement_boundary(cursor_.previous(), tok, next);

                if (config.ignore_standalone_modifiers_as_boundaries && type == TokenType::At)
                {
                    if (!is_at_any_declaration())
                    {
                        is_boundary = false;
                    }
                }

                if (is_boundary) return true;
            }

            if (config.stop_at_any_newline && tok.line > cursor_.previous().line)
            {
                return true;
            }

            if (config.stop_at_currently_tracked_closers && is_active_closer(type)) return true;

            if (config.stop_at_currently_tracked_sync_tokens && is_in_sync_set(type)) return true;

            if (config.custom_stop_predicate && config.custom_stop_predicate(tok, next)) return true;

            if (config.stop_early_if_unbalanced_blocks_detected)
            {
                if (is_missing_closing_brace() && (
                    is_at_top_level_declaration() || tok.type == TokenType::Return ||
                    TokenTraits::is_statement_start(tok, next) ||
                    (tok.line > cursor_.previous().line &&
                        TokenTraits::is_expression_statement_start(tok, next))))
                {
                    return true;
                }
            }

            return false;
        });
    }

    void Parser::synchronize_to_closer(TokenType closing_token)
    {
        recover([&](TokenType t, int depth)
        {
            if (depth == 0)
            {
                if (t == closing_token) return true;
                if (TokenTraits::is_grouping_closer(t))
                {
                    if (cursor_.peek(1).type == TokenType::Comma) cursor_.advance();
                    return true;
                }
                if (TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type)) return true;
                if (cursor_.peek().line > cursor_.previous().line &&
                    TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type))
                    return true;
                if (t == TokenType::Then || t == TokenType::Else || t == TokenType::Case || t == TokenType::Default)
                    return true;
            }
            return false;
        });
    }

    void Parser::synchronize_and_consume_closer(TokenType expected_closer)
    {
        synchronize_to_closer(expected_closer);
        if (cursor_.check(expected_closer) ||
            (TokenTraits::is_grouping_closer(cursor_.peek().type) && !is_active_closer(cursor_.peek().type)))
        {
            cursor_.advance();
        }
    }
}
