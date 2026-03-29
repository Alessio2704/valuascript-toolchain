#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    const Token &Parser::consume_identifier(ValuascriptErrorCode fallback_err) {
        if (cursor_.check(TokenType::Identifier)) {
            return cursor_.advance();
        }

        if (is_reserved_keyword(cursor_.peek())) {
            TokenType next = cursor_.peek(1).type;
            if (acts_like_identifier(cursor_.peek(), next)) {
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
            [this, key_err, colon_err]() {
                Token key_token = consume_identifier(key_err);
                cursor_.consume(TokenType::Colon, colon_err);
                auto val = parse_expression();

                if (is_expression_start(cursor_.peek().type) && cursor_.peek(1).type != TokenType::Colon) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
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
}
