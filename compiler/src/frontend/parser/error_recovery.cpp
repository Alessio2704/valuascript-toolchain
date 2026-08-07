#include "error_recovery.h"

namespace valuascript::compiler
{
    CloserTracker::CloserTracker(ParserContext& c, TokenType t) : ctx(c)
    {
        ctx.active_closers.push_back(t);
    }

    CloserTracker::~CloserTracker()
    {
        ctx.active_closers.pop_back();
    }

    SyncSetTracker::SyncSetTracker(ParserContext& c, const std::vector<TokenType>& tokens) : ctx(c)
    {
        previous_size = ctx.sync_set.size();
        ctx.sync_set.insert(ctx.sync_set.end(), tokens.begin(), tokens.end());
    }

    SyncSetTracker::~SyncSetTracker()
    {
        ctx.sync_set.resize(previous_size);
    }

    void ErrorRecovery::recover(ParserContext& ctx, const SyncPredicate& stop_condition)
    {
        int depth = 0;
        while (!ctx.cursor.is_at_end())
        {
            TokenType t = ctx.cursor.peek().type;
            if (stop_condition(t, depth)) return;
            if (TokenTraits::is_grouping_opener(t)) depth++;
            else if (TokenTraits::is_grouping_closer(t)) depth--;
            ctx.cursor.advance();
            if (depth < 0) depth = 0;
        }
    }

    void ErrorRecovery::synchronize_with(ParserContext& ctx, const RecoveryConfig& config)
    {
        recover(ctx, [&](TokenType type, int depth)
        {
            if (config.has(RecoveryOptions::SkipNestedGroupings) && depth > 0) return false;
            for (TokenType t : config.stop_tokens) if (type == t) return true;

            const Token& tok = ctx.cursor.peek();
            const TokenType next = ctx.cursor.peek(1).type;

            if (type == TokenType::Return && ctx.is_active_closer(TokenType::RightBrace))
            {
                if (config.has(RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp) ||
                    config.has(RecoveryOptions::StopAtBoundaryRespectingDanglingOp))
                    return true;
            }

            if (config.has(RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp))
            {
                if (TokenTraits::is_statement_start(tok, next) || (tok.line > ctx.cursor.previous().line &&
                    TokenTraits::is_expression_statement_start(tok, next)))
                    return true;
            }

            if (config.has(RecoveryOptions::StopAtBoundaryRespectingDanglingOp))
            {
                bool is_boundary = TokenTraits::is_statement_start(tok, next) ||
                    TokenTraits::is_newline_statement_boundary(ctx.cursor.previous(), tok, next);
                if (config.has(RecoveryOptions::IgnoreStandaloneModifiersAsBoundaries) && type == TokenType::At)
                {
                    if (!ctx.is_at_any_declaration()) is_boundary = false;
                }
                if (is_boundary) return true;
            }

            if (config.has(RecoveryOptions::StopAtNewline) && tok.line > ctx.cursor.previous().line) return true;
            if (config.has(RecoveryOptions::StopAtTrackedClosers) && ctx.is_active_closer(type)) return true;
            if (config.has(RecoveryOptions::StopAtTrackedSyncTokens) && ctx.is_in_sync_set(type)) return true;
            if (config.custom_stop_predicate && config.custom_stop_predicate(tok, next)) return true;

            if (config.has(RecoveryOptions::StopEarlyIfUnbalancedBlocks))
            {
                if (ctx.is_missing_closing_brace() && (ctx.is_at_top_level_declaration() || tok.type ==
                    TokenType::Return ||
                    TokenTraits::is_statement_start(tok, next) || (tok.line > ctx.cursor.previous().line &&
                        TokenTraits::is_expression_statement_start(tok, next))))
                    return true;
            }
            return false;
        });
    }

    void ErrorRecovery::synchronize_to_closer(ParserContext& ctx, TokenType closing_token)
    {
        recover(ctx, [&](TokenType t, int depth)
        {
            if (depth == 0)
            {
                if (t == closing_token) return true;
                if (TokenTraits::is_grouping_closer(t))
                {
                    if (ctx.cursor.peek(1).type == TokenType::Comma) ctx.cursor.advance();
                    return true;
                }
                if (TokenTraits::is_statement_start(ctx.cursor.peek(), ctx.cursor.peek(1).type)) return true;
                if (ctx.cursor.peek().line > ctx.cursor.previous().line && TokenTraits::is_expression_statement_start(
                    ctx.cursor.peek(), ctx.cursor.peek(1).type))
                    return true;
                if (t == TokenType::Then || t == TokenType::Else || t == TokenType::Case || t == TokenType::Default)
                    return true;
            }
            return false;
        });
    }

    void ErrorRecovery::synchronize_and_consume_closer(ParserContext& ctx, TokenType expected_closer)
    {
        synchronize_to_closer(ctx, expected_closer);
        if (ctx.cursor.check(expected_closer) || (TokenTraits::is_grouping_closer(ctx.cursor.peek().type) && !
            ctx.is_active_closer(ctx.cursor.peek().type)))
            ctx.cursor.advance();
    }
}
