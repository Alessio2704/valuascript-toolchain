#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    const Token &Parser::consume_identifier(ValuascriptErrorCode fallback_err, bool is_statement_context) {
        if (cursor_.check(TokenType::Identifier)) {
            return cursor_.advance();
        }

        if (is_reserved_keyword(cursor_.peek())) {
            TokenType next = cursor_.peek(1).type;

            bool acts_like_id = acts_like_identifier(cursor_.peek(), next);
            bool forms_statement = is_statement_start(cursor_.peek(), next);

            if (acts_like_id || !is_statement_context || !forms_statement) {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
                return cursor_.advance();
            }
        }

        return cursor_.consume(TokenType::Identifier, fallback_err);
    }

    void Parser::verify_statement_end() const {
        if (!cursor_.is_at_end() && cursor_.peek().line == cursor_.previous().line) {
            if (is_expression_start(cursor_.peek().type)) {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
            }
        }
    }

    std::vector<std::unique_ptr<Expression> > Parser::parse_expression_list(
        const TokenType closing_token,
        const std::optional<ValuascriptErrorCode> trailing_comma_err,
        const std::initializer_list<TokenType> panic_stops) {
        return parse_comma_separated_list<std::unique_ptr<Expression> >(
            closing_token,
            trailing_comma_err,
            ValuascriptErrorCode::MissingCommaOrOperatorBetweenExpressions,
            panic_stops,
            [this]() { return is_expression_start(cursor_.peek().type); },
            [this]() { return parse_expression(); }
        );
    }

    std::vector<std::pair<std::string, std::unique_ptr<Expression> > > Parser::parse_key_value_list(
        const TokenType closing_token,
        const ValuascriptErrorCode key_err,
        const ValuascriptErrorCode colon_err,
        const ValuascriptErrorCode missing_comma_err,
        const std::optional<ValuascriptErrorCode> trailing_comma_err,
        const std::initializer_list<TokenType> panic_stops) {
        return parse_comma_separated_list<std::pair<std::string, std::unique_ptr<Expression> > >(
            closing_token,
            trailing_comma_err,
            missing_comma_err,
            panic_stops,
            [this]() {
                const Token &tok = cursor_.peek();
                bool is_id_like = tok.type == TokenType::Identifier || acts_like_identifier(tok, cursor_.peek(1).type);
                return is_id_like && cursor_.peek(1).type == TokenType::Colon;
            },
            [this, key_err, colon_err, closing_token]() {
                Token key_token = consume_identifier(key_err, false);
                cursor_.consume(TokenType::Colon, colon_err);

                std::unique_ptr<Expression> val = nullptr;

                if (cursor_.check(TokenType::Comma) || cursor_.check(closing_token)) {
                    cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::InvalidExpression);
                } else {
                    val = parse_expression();
                    if (is_expression_start(cursor_.peek().type) && cursor_.peek(1).type != TokenType::Colon) {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                    }
                }

                return std::make_pair(key_token.lexeme, std::move(val));
            }
        );
    }

    void Parser::consume_unexpected_statement_gracefully() {
        Program dummy;
        try {
            std::vector<std::unique_ptr<Statement> > dummy_block;
            parse_statement_or_declaration(ParseContext::TopLevel, &dummy, dummy_block);
        } catch (const ParseSyncException &) {
        }
    }

    bool Parser::is_at_top_level_declaration() const {
        int offset = 0;

        while (cursor_.peek(offset).type != TokenType::EndOfFile && cursor_.peek(offset).type == TokenType::At) {
            offset++;
            if (cursor_.peek(offset).type == TokenType::EndOfFile) break;
            offset++;

            if (cursor_.peek(offset).type == TokenType::LeftParen) {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor_.peek(offset).type != TokenType::EndOfFile) {
                    if (cursor_.peek(offset).type == TokenType::LeftParen) depth++;
                    else if (cursor_.peek(offset).type == TokenType::RightParen) depth--;
                    offset++;
                }
            }
        }

        return is_top_level_only_declaration(cursor_.peek(offset).type);
    }

    bool Parser::is_at_any_declaration() const {
        int offset = 0;

        while (cursor_.peek(offset).type != TokenType::EndOfFile && cursor_.peek(offset).type == TokenType::At) {
            offset++;
            if (cursor_.peek(offset).type == TokenType::EndOfFile) break;
            offset++;

            if (cursor_.peek(offset).type == TokenType::LeftParen) {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor_.peek(offset).type != TokenType::EndOfFile) {
                    if (cursor_.peek(offset).type == TokenType::LeftParen) depth++;
                    else if (cursor_.peek(offset).type == TokenType::RightParen) depth--;
                    offset++;
                }
            }
        }

        TokenType t = cursor_.peek(offset).type;
        if (t == TokenType::Let || t == TokenType::Var) {
            if (cursor_.peek(offset + 1).type == TokenType::EndOfFile) return false;
            return cursor_.peek(offset + 1).type == TokenType::Identifier;
        }

        return is_top_level_only_declaration(t);
    }

    bool Parser::is_missing_closing_brace() const {
        int depth = 1;
        int offset = 0;

        while (true) {
            TokenType t = cursor_.peek(offset).type;

            if (t == TokenType::EndOfFile) {
                return true;
            }

            if (t == TokenType::LeftBrace) {
                depth++;
            } else if (t == TokenType::RightBrace) {
                depth--;
                if (depth == 0) {
                    return false;
                }
            }
            offset++;
        }
    }
}
