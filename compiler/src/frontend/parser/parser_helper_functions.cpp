#include "parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler
{
    const Token& Parser::consume_identifier(ValuascriptErrorCode fallback_err, bool allow_top_level_keywords,
                                            bool check_statement_boundary)
    {
        if (check_statement_boundary && cursor_.peek().line > cursor_.previous().line &&
            TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type))
        {
            cursor_.report_error(cursor_.peek(), fallback_err);
        }
        if (cursor_.check(TokenType::Identifier))
        {
            return cursor_.advance();
        }

        if (is_reserved_keyword(cursor_.peek()))
        {
            TokenType next = cursor_.peek(1).type;

            bool acts_like_id = TokenTraits::acts_like_identifier(cursor_.peek(), next);
            bool forms_statement = TokenTraits::is_statement_start(cursor_.peek(), next) ||
            (check_statement_boundary && cursor_.peek().line > cursor_.previous().line &&
                TokenTraits::is_expression_statement_start(cursor_.peek(), next));

            if (acts_like_id || !allow_top_level_keywords || !forms_statement)
            {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
                return cursor_.advance();
            }
        }

        return cursor_.consume(TokenType::Identifier, fallback_err, false);
    }

    void Parser::verify_statement_end() const
    {
        if (!cursor_.is_at_end() && cursor_.peek().line == cursor_.previous().line)
        {
            if (TokenTraits::is_expression_start(cursor_.peek().type))
            {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
            }
        }
    }

    std::vector<std::unique_ptr<Expression>> Parser::parse_expression_list(
        const TokenType closing_token,
        const std::optional<ValuascriptErrorCode> trailing_comma_err,
        const std::vector<TokenType> recovery_boundaries)
    {
        return parse_list<std::unique_ptr<Expression>>(
            closing_token,
            trailing_comma_err,
            ValuascriptErrorCode::MissingCommaOrOperatorBetweenExpressions,
            recovery_boundaries,
            [this]() { return TokenTraits::is_expression_start(cursor_.peek().type); },
            [this]() { return parse_expression(); }
        );
    }

    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> Parser::parse_key_value_list(
        const TokenType closing_token,
        const ValuascriptErrorCode key_err,
        const ValuascriptErrorCode colon_err,
        const ValuascriptErrorCode missing_comma_err,
        const std::optional<ValuascriptErrorCode> trailing_comma_err,
        const std::vector<TokenType> recovery_boundaries)
    {
        return parse_list<std::pair<std::string, std::unique_ptr<Expression>>>(
            closing_token,
            trailing_comma_err,
            missing_comma_err,
            recovery_boundaries,
            [this]()
            {
                const Token& tok = cursor_.peek();
                bool is_id_like = tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(
                    tok, cursor_.peek(1).type);
                return is_id_like && cursor_.peek(1).type == TokenType::Colon;
            }, [this, key_err, colon_err, closing_token]()
            {
                bool key_failed = false;
                Token key_token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
                try
                {
                    key_token = consume_identifier(key_err, false);
                }
                catch (const ParseSyncException&)
                {
                    key_failed = true;
                    while (!cursor_.is_at_end() && !cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::Comma)
                        && !cursor_.check(closing_token))
                    {
                        if (TokenTraits::is_newline_statement_boundary(cursor_.previous(), cursor_.peek(),
                                                                       cursor_.peek(1).type))
                            break;
                        if (is_in_sync_set(cursor_.peek().type)) break;
                        cursor_.advance();
                    }
                }

                bool has_colon = false;
                if (cursor_.check(TokenType::Colon))
                {
                    has_colon = true;
                    cursor_.advance();
                }
                else
                {
                    if (!key_failed)
                    {
                        if (!cursor_.check(TokenType::Comma) && !cursor_.check(closing_token))
                        {
                            cursor_.consume(TokenType::Colon, colon_err, true);
                        }
                        else
                        {
                            cursor_.report_error_no_panic(cursor_.peek(), colon_err, true);
                        }
                    }
                }

                std::unique_ptr<Expression> val = nullptr;

                if (cursor_.check(TokenType::Comma) || cursor_.check(closing_token))
                {
                    if (has_colon)
                    {
                        cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::InvalidExpression, true);
                    }
                }
                else
                {
                    try
                    {
                        val = parse_expression();
                        if ((TokenTraits::is_expression_start(cursor_.peek().type) ||
                                TokenTraits::is_binary_operator(cursor_.peek().type)) && cursor_.peek(1).type !=
                            TokenType::Colon)
                        {
                            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                        }
                    }
                    catch (const ParseSyncException&)
                    {
                        while (!cursor_.is_at_end() && !cursor_.check(TokenType::Comma) && !cursor_.
                            check(closing_token))
                        {
                            if (TokenTraits::is_newline_statement_boundary(
                                cursor_.previous(), cursor_.peek(), cursor_.peek(1).type))
                                break;
                            if (is_in_sync_set(cursor_.peek().type)) break;
                            cursor_.advance();
                        }
                    }
                }

                return std::make_pair(key_token.lexeme, std::move(val));
            }
        );
    }

    void Parser::consume_unexpected_statement_gracefully()
    {
        bool prev_suppress = cursor_.get_suppress_errors();
        cursor_.set_suppress_errors(true);

        Program dummy;
        try
        {
            std::vector<std::unique_ptr<Statement>> dummy_block;
            parse_statement_or_declaration(ParseContext::TopLevel, &dummy, dummy_block);
        }
        catch (const ParseSyncException&)
        {
        }

        cursor_.set_suppress_errors(prev_suppress);
    }

    TokenType Parser::peek_past_modifiers() const
    {
        int offset = 0;
        while (cursor_.peek(offset).type == TokenType::At)
        {
            offset++;
            if (cursor_.peek(offset).type == TokenType::EndOfFile) break;
            offset++;

            if (cursor_.peek(offset).type == TokenType::LeftParen)
            {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor_.peek(offset).type != TokenType::EndOfFile)
                {
                    if (cursor_.peek(offset).type == TokenType::LeftParen) depth++;
                    else if (cursor_.peek(offset).type == TokenType::RightParen) depth--;
                    offset++;
                }
            }
        }
        return cursor_.peek(offset).type;
    }

    bool Parser::is_at_top_level_declaration() const
    {
        return TokenTraits::is_top_level_only_declaration(peek_past_modifiers());
    }

    bool Parser::is_at_any_declaration() const
    {
        TokenType t = peek_past_modifiers();
        if (t == TokenType::Let) return true;
        return TokenTraits::is_top_level_only_declaration(t);
    }

    bool Parser::is_missing_closing_brace() const
    {
        int depth = 1;
        int offset = 0;

        while (true)
        {
            TokenType t = cursor_.peek(offset).type;

            if (t == TokenType::EndOfFile) return true;

            if (t == TokenType::LeftBrace)
            {
                depth++;
            }
            else if (t == TokenType::RightBrace)
            {
                depth--;
                if (depth == 0) return false;
            }
            offset++;
        }
    }
}
