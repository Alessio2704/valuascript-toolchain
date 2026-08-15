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

    namespace
    {
        bool can_end_expression(TokenType type)
        {
            switch (type)
            {
            case TokenType::RightParen:
            case TokenType::RightBracket:
            case TokenType::RightBrace:
            case TokenType::Identifier:
            case TokenType::Number:
            case TokenType::String:
            case TokenType::True:
            case TokenType::False:
            case TokenType::Self: return true;
            default: return false;
            }
        }
    }

    bool ParserContext::looks_like_reassignment() const
    {
        const Token& first = cursor.peek();
        if (!TokenTraits::is_identifier_start(first) &&
            first.type != TokenType::Self &&
            first.type != TokenType::LeftParen &&
            first.type != TokenType::LeftBracket &&
            first.type != TokenType::LeftBrace)
        {
            return false;
        }

        int depth = 0;
        size_t offset = 0;

        while (true)
        {
            const Token& tok = cursor.peek(offset);
            if (tok.type == TokenType::EndOfFile) break;

            if (offset > 0)
            {
                if (tok.type == TokenType::Case || tok.type == TokenType::Default ||
                    TokenTraits::is_statement_start(tok, cursor.peek(offset + 1).type))
                {
                    break;
                }

                if (tok.line > cursor.peek(offset - 1).line)
                {
                    if (TokenTraits::is_expression_statement_start(tok, cursor.peek(offset + 1).type) ||
                        tok.type == TokenType::LeftBracket || tok.type == TokenType::LeftParen ||
                        tok.type == TokenType::Return ||
                        (tok.type == TokenType::At && is_at_any_declaration()))
                    {
                        break;
                    }
                }
            }

            if (TokenTraits::is_grouping_opener(tok.type)) depth++;
            else if (TokenTraits::is_grouping_closer(tok.type))
            {
                depth--;
                if (depth < 0) break;
                if (depth == 0 && tok.type == TokenType::RightBrace) break;
            }

            if (tok.type == TokenType::Assign)
            {
                return true;
            }

            if (offset > 0 && (tok.type == TokenType::Identifier || TokenTraits::is_identifier_start(tok)) &&
                can_end_expression(cursor.peek(offset - 1).type))
            {
                break;
            }

            if (depth == 0)
            {
                if (tok.type == TokenType::Comma || tok.type == TokenType::Colon ||
                    tok.type == TokenType::At || tok.type == TokenType::RightBrace)
                    break;
            }

            offset++;
        }

        return false;
    }

    const Token& ParserContext::consume_identifier(ParserErrorCode fallback_err, bool allow_top_level_keywords,
                                                   bool check_statement_boundary)
    {
        if (cursor.previous().type == TokenType::Dot)
        {
            const Token& peek_tok = cursor.peek();
            TokenType next = cursor.peek(1).type;

            bool is_newline = peek_tok.line > cursor.previous().line;
            bool is_same_line_clause_boundary = false;

            if (!is_newline && is_reserved_keyword(peek_tok))
            {
                if (peek_tok.type == TokenType::Default || peek_tok.type == TokenType::Case)
                {
                    if (next == TokenType::Arrow || next == TokenType::Colon) is_same_line_clause_boundary = true;
                }
                else if (peek_tok.type == TokenType::Then)
                {
                    if (cursor.peek(1).line == peek_tok.line && TokenTraits::is_expression_start(next))
                    {
                        is_same_line_clause_boundary = true;
                    }
                }
                else if (peek_tok.type == TokenType::Else)
                {
                    if (cursor.peek(1).line == peek_tok.line && (TokenTraits::is_expression_start(next) || next ==
                        TokenType::If))
                    {
                        is_same_line_clause_boundary = true;
                    }
                }
            }

            if (is_same_line_clause_boundary || (is_newline && (is_reserved_keyword(peek_tok) ||
                looks_like_reassignment())))
            {
                cursor.report_error(cursor.peek(), fallback_err);
            }
        }
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
                if (cursor.peek().line == cursor.previous().line && !TokenTraits::is_statement_start(
                    cursor.peek(), next))
                {
                    acts_like_id = true;
                }
            }

            bool forms_statement = TokenTraits::is_statement_start(cursor.peek(), next) || (check_statement_boundary &&
                cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), next));

            if (cursor.previous().type == TokenType::Dot || acts_like_id || !allow_top_level_keywords || !
                forms_statement)
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
        if (t == TokenType::Return && is_active_closer(TokenType::RightBrace)) return true;
        return TokenTraits::is_top_level_only_declaration(t);
    }

    bool ParserContext::is_missing_closing_brace() const
    {
        int active_braces = static_cast<int>(std::count(active_closers.begin(), active_closers.end(), TokenType::RightBrace));
        int depth = active_braces > 0 ? active_braces : 1;
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
