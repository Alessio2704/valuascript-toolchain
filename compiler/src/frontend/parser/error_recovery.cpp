#include "error_recovery.h"

namespace valuascript::compiler
{
    CloserTracker::CloserTracker(ParserContext& c, TokenType t, ContainerKind k) : ctx(c)
    {
        ctx.active_closers.push_back(CloserFrame{.type = t, .kind = k});
    }

    CloserTracker::~CloserTracker()
    {
        if (!ctx.active_closers.empty())
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
                if (ctx.is_at_any_declaration() || TokenTraits::is_statement_start(tok, next) || (tok.line > ctx.cursor.previous().line &&
                    TokenTraits::is_expression_statement_start(tok, next)))
                    return true;
            }

            if (config.has(RecoveryOptions::StopAtBoundaryRespectingDanglingOp))
            {
                bool is_boundary = ctx.is_at_any_declaration() || TokenTraits::is_statement_start(tok, next) ||
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
                if (ctx.is_missing_closing_brace() && (ctx.is_at_top_level_declaration() || ctx.is_at_any_declaration() || tok.type ==
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
            if (TokenTraits::is_identifier_start(ctx.cursor.peek()) &&
                (ctx.cursor.peek(1).type == TokenType::Colon || ctx.cursor.peek(1).type == TokenType::Assign ||
                 ctx.cursor.peek(1).type == TokenType::Arrow || ctx.cursor.peek(1).type == TokenType::Dot ||
                 ctx.cursor.peek(1).type == TokenType::LeftParen || ctx.cursor.peek(1).type == TokenType::LeftBracket))
                return true;
            if (ctx.looks_like_reassignment()) return true;
            if (t == TokenType::Comma && TokenTraits::is_identifier_start(ctx.cursor.peek(1)) &&
                (ctx.cursor.peek(2).type == TokenType::Colon || ctx.cursor.peek(2).type == TokenType::Assign || ctx.cursor.peek(2).type == TokenType::Arrow))
                return true;
            if (TokenTraits::is_grouping_closer(t))
                {
                    if (ctx.cursor.peek(1).type == TokenType::Comma) ctx.cursor.advance();
                    return true;
                }
                if (TokenTraits::is_statement_start(ctx.cursor.peek(), ctx.cursor.peek(1).type)) return true;
                if (TokenTraits::is_newline_statement_boundary(ctx.cursor.previous(), ctx.cursor.peek(), ctx.cursor.peek(1).type)) return true;
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

    bool ErrorRecovery::should_yield_closer_to_parent(ParserContext& ctx, TokenType closer_type)
    {
        if (!ctx.cursor.check(closer_type))
            return false;

        size_t available_closers = 0;
        while (ctx.cursor.peek(available_closers).type == closer_type)
            available_closers++;

        const Token& boundary = ctx.cursor.peek(available_closers);
        const Token& after_boundary = ctx.cursor.peek(available_closers + 1);
        const Token& last_closer = ctx.cursor.peek(available_closers - 1);

        bool in_param_or_dict = false;
        bool has_switch_target = false;
        size_t switch_target_idx = 0;
        bool has_switch_body = false;
        size_t switch_body_idx = 0;
        size_t sibling_container_idx = 0;
        bool has_sibling_container = false;
        for (size_t i = ctx.active_closers.size(); i > 0; --i)
        {
            auto kind = ctx.active_closers[i - 1].kind;
            if (kind == ContainerKind::FunctionParameters ||
                kind == ContainerKind::DictionaryLiteral)
                in_param_or_dict = true;
            if (!has_switch_target && kind == ContainerKind::SwitchTarget)
            {
                has_switch_target = true;
                switch_target_idx = i - 1;
            }
            if (!has_switch_body && kind == ContainerKind::SwitchBody)
            {
                has_switch_body = true;
                switch_body_idx = i - 1;
            }
            if (!has_sibling_container &&
                (kind == ContainerKind::CallArguments ||
                 kind == ContainerKind::ModifierArguments ||
                 kind == ContainerKind::FunctionParameters ||
                 kind == ContainerKind::DictionaryLiteral))
            {
                has_sibling_container = true;
                sibling_container_idx = i - 1;
            }
        }

        auto scan_past_modifiers = [&](size_t offset) -> size_t
        {
            size_t mod_offset = offset;
            while (ctx.cursor.peek(mod_offset).type == TokenType::At)
            {
                mod_offset++;
                if (ctx.cursor.peek(mod_offset).type == TokenType::EndOfFile) break;
                mod_offset++;
                if (ctx.cursor.peek(mod_offset).type == TokenType::LeftParen)
                {
                    int paren_depth = 1;
                    mod_offset++;
                    while (paren_depth > 0 && ctx.cursor.peek(mod_offset).type != TokenType::EndOfFile)
                    {
                        if (ctx.cursor.peek(mod_offset).type == TokenType::LeftParen) paren_depth++;
                        else if (ctx.cursor.peek(mod_offset).type == TokenType::RightParen) paren_depth--;
                        mod_offset++;
                    }
                }
            }
            return mod_offset;
        };

        size_t after_sibling_offset = available_closers + 1;
        if (boundary.type == TokenType::Comma && after_boundary.type == TokenType::At)
        {
            after_sibling_offset = scan_past_modifiers(available_closers + 1);
        }
        TokenType after_mods_type = ctx.cursor.peek(after_sibling_offset).type;
        TokenType after_mods_next_type = ctx.cursor.peek(after_sibling_offset + 1).type;

        bool is_sibling_boundary =
            boundary.type == TokenType::Comma &&
            (after_mods_type == TokenType::Identifier || TokenTraits::acts_like_identifier(ctx.cursor.peek(after_sibling_offset), after_mods_next_type)) &&
            (after_mods_next_type == TokenType::Assign || after_mods_next_type == TokenType::Colon);

        bool is_switch_case_modifier = false;
        if (has_switch_body && boundary.type == TokenType::At)
        {
            size_t past_mod = scan_past_modifiers(available_closers);
            TokenType past_type = ctx.cursor.peek(past_mod).type;
            if (past_type == TokenType::Case || past_type == TokenType::Default)
            {
                is_switch_case_modifier = true;
            }
        }

        bool is_stmt_boundary =
            boundary.type == TokenType::Arrow ||
            boundary.type == TokenType::Assign ||
            (boundary.type == TokenType::LeftBrace && ctx.expr_depth == 0 && ctx.key_value_container_depth == 0) ||
            boundary.type == TokenType::Case ||
            boundary.type == TokenType::Default ||
            boundary.type == TokenType::Return ||
            boundary.type == TokenType::EndOfFile ||
            (boundary.type != TokenType::At && TokenTraits::is_statement_start(boundary, after_boundary.type)) ||
            (boundary.type == TokenType::At && !in_param_or_dict && TokenTraits::is_statement_start(boundary, after_boundary.type)) ||
            (boundary.type == TokenType::Identifier && after_boundary.type == TokenType::Assign) ||
            TokenTraits::is_newline_statement_boundary(last_closer, boundary, after_boundary.type, closer_type == TokenType::Greater);

        bool is_switch_brace = boundary.type == TokenType::LeftBrace &&
                               has_switch_target &&
                               (after_boundary.type == TokenType::Case ||
                                after_boundary.type == TokenType::Default ||
                                after_boundary.type == TokenType::At);

        bool is_target_boundary =
            (closer_type == TokenType::RightParen &&
             boundary.type == TokenType::Identifier &&
             after_boundary.type == TokenType::Colon);

        size_t min_idx = ctx.expr_closers_baseline;
        if (is_sibling_boundary && has_sibling_container)
            min_idx = sibling_container_idx + 1;
        else if (has_switch_body && (boundary.type == TokenType::Case || boundary.type == TokenType::Default || is_switch_case_modifier))
            min_idx = switch_body_idx + 1;
        else if (boundary.type == TokenType::Arrow || boundary.type == TokenType::EndOfFile || is_stmt_boundary)
            min_idx = 0;
        else if (is_target_boundary)
            min_idx = 0;
        else if (is_switch_brace)
            min_idx = switch_target_idx;

        size_t active_closers = 0;
        for (size_t i = ctx.active_closers.size(); i > min_idx; --i)
        {
            size_t idx = i - 1;
            const CloserFrame& frame = ctx.active_closers[idx];
            if (frame.type != closer_type)
                break;
            if (closer_type == TokenType::RightParen)
            {
                if (frame.kind == ContainerKind::FunctionParameters &&
                    !(boundary.type == TokenType::Arrow ||
                      (boundary.type == TokenType::LeftBrace && ctx.expr_depth == 0 && ctx.key_value_container_depth == 0)))
                    continue;
            }
            if (closer_type == TokenType::RightBrace)
            {
                if (frame.kind == ContainerKind::Block && is_stmt_boundary && boundary.type != TokenType::EndOfFile)
                    continue;
            }
            active_closers++;
        }

        if (active_closers > 1)
        {
            if (available_closers < active_closers)
            {
                if (is_stmt_boundary || is_sibling_boundary || is_target_boundary || is_switch_brace ||
                    (closer_type == TokenType::RightBrace && boundary.type == TokenType::Comma) ||
                    ctx.is_active_closer(boundary.type))
                    return true;
            }
        }

        return false;
    }

    namespace
    {
        TokenType get_matching_opener(TokenType closer)
        {
            switch (closer)
            {
                case TokenType::RightParen: return TokenType::LeftParen;
                case TokenType::RightBracket: return TokenType::LeftBracket;
                case TokenType::RightBrace: return TokenType::LeftBrace;
                case TokenType::Greater: return TokenType::Less;
                default: return TokenType::EndOfFile;
            }
        }
    }

    bool ErrorRecovery::is_unclosed_before_parent_boundary(ParserContext& ctx, TokenType closer_type)
    {
        size_t start_idx = 0;
        for (size_t i = ctx.active_closers.size(); i > 0; --i)
        {
            if (ctx.active_closers[i - 1] != closer_type)
            {
                start_idx = i;
                break;
            }
        }

        size_t active_closer_count = 0;
        for (size_t i = start_idx; i < ctx.active_closers.size(); ++i)
        {
            if (ctx.active_closers[i] == closer_type)
                active_closer_count++;
        }

        if (active_closer_count == 0) return false;

        TokenType opener_type = get_matching_opener(closer_type);
        size_t available_closers = 0;
        size_t nesting = 0;
        size_t offset = 0;
        while (true)
        {
            const Token& tok = ctx.cursor.peek(offset);
            if (tok.type == TokenType::EndOfFile)
                break;

            if (opener_type != TokenType::EndOfFile && tok.type == opener_type)
            {
                nesting++;
            }
            else if (tok.type == closer_type)
            {
                if (nesting > 0)
                    nesting--;
                else
                    available_closers++;
            }
            else if (nesting == 0)
            {
                if (tok.type != closer_type && ctx.is_active_closer(tok.type))
                    break;
                if (tok.line > ctx.cursor.peek().line && (TokenTraits::is_statement_start(tok, ctx.cursor.peek(offset + 1).type) || ctx.is_at_any_declaration() || ctx.looks_like_reassignment()))
                    break;
            }
            offset++;
        }

        return available_closers < active_closer_count;
    }
}
