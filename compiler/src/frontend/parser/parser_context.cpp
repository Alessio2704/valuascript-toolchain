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

    bool ParserContext::looks_like_reassignment() const
    {
        int depth = 0;
        size_t offset = 0;
        size_t start_line = cursor.peek().line;

        while (true)
        {
            const Token& tok = cursor.peek(offset);
            if (tok.type == TokenType::EndOfFile || tok.line > start_line) break;

            if (TokenTraits::is_grouping_opener(tok.type)) depth++;
            else if (TokenTraits::is_grouping_closer(tok.type))
            {
                depth--;
                if (depth < 0) depth = 0;
            }
            else if (depth == 0 && tok.type == TokenType::Assign) return true;

            offset++;
        }
        return false;
    }

    const Token& ParserContext::consume_identifier(ParserErrorCode fallback_err, bool allow_top_level_keywords,
                                                   bool check_statement_boundary)
    {
        if (check_statement_boundary && cursor.peek().line > cursor.previous().line &&
            TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type))
        {
            bool is_dot_override = false;
            if (cursor.previous().type == TokenType::Dot)
            {
                is_dot_override = true;
                if (looks_like_reassignment()) is_dot_override = false;
            }

            if (!is_dot_override) cursor.report_error(cursor.peek(), fallback_err);
        }

        if (cursor.check(TokenType::Identifier)) return cursor.advance();

        if (is_reserved_keyword(cursor.peek()))
        {
            TokenType next = cursor.peek(1).type;
            bool acts_like_id = TokenTraits::acts_like_identifier(cursor.peek(), next);

            if (cursor.previous().type == TokenType::Dot)
            {
                if (cursor.peek().line == cursor.previous().line)
                {
                    acts_like_id = true;
                }
                else if (next != TokenType::Identifier)
                {
                    acts_like_id = true;
                }
            }

            bool forms_statement = TokenTraits::is_statement_start(cursor.peek(), next) || (check_statement_boundary &&
                cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), next));

            if (acts_like_id || !allow_top_level_keywords || !forms_statement)
            {
                cursor.report_error_no_panic(cursor.peek(), ParserErrorCode::ReservedKeywordAsIdentifier, true);
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

    void ParserContext::reject_modifiers(const std::vector<Modifier>& modifiers, ParserErrorCode error_code) const
    {
        if (modifiers.empty()) return;
        SourceSpan span = cursor.combine_spans(modifiers.front().span, modifiers.back().span);
        cursor.report_error_no_panic(span, error_code);
    }
}
