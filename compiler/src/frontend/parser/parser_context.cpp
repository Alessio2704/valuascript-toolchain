#include "parser_context.h"
#include "token/reserved_keyword_lookup.h"
#include <algorithm>

namespace valuascript::compiler
{
    ParserContext::ParserContext(TokenCursor c) : cursor(std::move(c))
    {
    }

    bool ParserContext::is_active_closer(TokenType type) const
    {
        return std::find(active_closers.begin(), active_closers.end(), type) != active_closers.end();
    }

    bool ParserContext::is_in_sync_set(TokenType type) const
    {
        return std::find(sync_set.begin(), sync_set.end(), type) != sync_set.end();
    }

    void ParserContext::recover(const SyncPredicate& stop_condition)
    {
        int depth = 0;
        while (!cursor.is_at_end())
        {
            TokenType t = cursor.peek().type;
            if (stop_condition(t, depth)) return;
            if (TokenTraits::is_grouping_opener(t)) depth++;
            else if (TokenTraits::is_grouping_closer(t)) depth--;
            cursor.advance();
            if (depth < 0) depth = 0;
        }
    }

    void ParserContext::synchronize_with(const RecoveryConfig& config)
    {
        recover([&](TokenType type, int depth)
        {
            if (config.has(RecoveryOptions::SkipNestedGroupings) && depth > 0) return false;
            for (TokenType t : config.stop_tokens) if (type == t) return true;

            const Token& tok = cursor.peek();
            const TokenType next = cursor.peek(1).type;

            if (type == TokenType::Return && is_active_closer(TokenType::RightBrace))
            {
                if (config.has(RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp) ||
                    config.has(RecoveryOptions::StopAtBoundaryRespectingDanglingOp))
                    return true;
            }

            if (config.has(RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp))
            {
                if (TokenTraits::is_statement_start(tok, next) || (tok.line > cursor.previous().line &&
                    TokenTraits::is_expression_statement_start(tok, next)))
                    return true;
            }

            if (config.has(RecoveryOptions::StopAtBoundaryRespectingDanglingOp))
            {
                bool is_boundary = TokenTraits::is_statement_start(tok, next) ||
                    TokenTraits::is_newline_statement_boundary(cursor.previous(), tok, next);
                if (config.has(RecoveryOptions::IgnoreStandaloneModifiersAsBoundaries) && type == TokenType::At)
                {
                    if (!is_at_any_declaration()) is_boundary = false;
                }
                if (is_boundary) return true;
            }

            if (config.has(RecoveryOptions::StopAtNewline) && tok.line > cursor.previous().line) return true;
            if (config.has(RecoveryOptions::StopAtTrackedClosers) && is_active_closer(type)) return true;
            if (config.has(RecoveryOptions::StopAtTrackedSyncTokens) && is_in_sync_set(type)) return true;
            if (config.custom_stop_predicate && config.custom_stop_predicate(tok, next)) return true;

            if (config.has(RecoveryOptions::StopEarlyIfUnbalancedBlocks))
            {
                if (is_missing_closing_brace() && (is_at_top_level_declaration() || tok.type == TokenType::Return ||
                    TokenTraits::is_statement_start(tok, next) || (tok.line > cursor.previous().line &&
                        TokenTraits::is_expression_statement_start(tok, next))))
                    return true;
            }

            return false;
        });
    }

    void ParserContext::synchronize_to_closer(TokenType closing_token)
    {
        recover([&](TokenType t, int depth)
        {
            if (depth == 0)
            {
                if (t == closing_token) return true;
                if (TokenTraits::is_grouping_closer(t))
                {
                    if (cursor.peek(1).type == TokenType::Comma) cursor.advance();
                    return true;
                }
                if (TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type)) return true;
                if (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), cursor.peek(1).type))
                    return true;
                if (t == TokenType::Then || t == TokenType::Else || t == TokenType::Case || t == TokenType::Default)
                    return true;
            }
            return false;
        });
    }

    void ParserContext::synchronize_and_consume_closer(TokenType expected_closer)
    {
        synchronize_to_closer(expected_closer);
        if (cursor.check(expected_closer) || (TokenTraits::is_grouping_closer(cursor.peek().type) && !
            is_active_closer(cursor.peek().type)))
            cursor.advance();
    }

    const Token& ParserContext::consume_identifier(ValuascriptErrorCode fallback_err, bool allow_top_level_keywords,
                                                   bool check_statement_boundary)
    {
        if (check_statement_boundary && cursor.peek().line > cursor.previous().line &&
            TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type))
        {
            cursor.report_error(cursor.peek(), fallback_err);
        }
        if (cursor.check(TokenType::Identifier)) return cursor.advance();

        if (is_reserved_keyword(cursor.peek()))
        {
            TokenType next = cursor.peek(1).type;
            bool acts_like_id = TokenTraits::acts_like_identifier(cursor.peek(), next);
            bool forms_statement = TokenTraits::is_statement_start(cursor.peek(), next) || (check_statement_boundary &&
                cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), next));

            if (acts_like_id || !allow_top_level_keywords || !forms_statement)
            {
                cursor.report_error_no_panic(cursor.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
                return cursor.advance();
            }
        }
        return cursor.consume(TokenType::Identifier, fallback_err, false);
    }

    TokenType ParserContext::peek_past_modifiers() const
    {
        size_t offset = 0;
        while (cursor.peek(offset).type == TokenType::At)
        {
            offset++;
            if (cursor.peek(offset).type == TokenType::EndOfFile) break;
            offset++;
            if (cursor.peek(offset).type == TokenType::LeftParen)
            {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor.peek(offset).type != TokenType::EndOfFile)
                {
                    if (cursor.peek(offset).type == TokenType::LeftParen) depth++;
                    else if (cursor.peek(offset).type == TokenType::RightParen) depth--;
                    offset++;
                }
            }
        }
        return cursor.peek(offset).type;
    }

    bool ParserContext::is_at_top_level_declaration() const
    {
        return TokenTraits::is_top_level_only_declaration(peek_past_modifiers());
    }

    bool ParserContext::is_at_any_declaration() const
    {
        TokenType t = peek_past_modifiers();
        if (t == TokenType::Let) return true;
        return TokenTraits::is_top_level_only_declaration(t);
    }

    bool ParserContext::is_missing_closing_brace() const
    {
        int depth = 1;
        size_t offset = 0;
        while (true)
        {
            TokenType t = cursor.peek(offset).type;
            if (t == TokenType::EndOfFile) return true;
            if (t == TokenType::LeftBrace) depth++;
            else if (t == TokenType::RightBrace)
            {
                depth--;
                if (depth == 0) return false;
            }
            offset++;
        }
    }

    void ParserContext::reject_modifiers(const std::vector<Modifier>& modifiers, ValuascriptErrorCode error_code) const
    {
        if (modifiers.empty()) return;
        SourceSpan span = cursor.combine_spans(modifiers.front().span, modifiers.back().span);
        cursor.report_error_no_panic(span, error_code);
    }
}
