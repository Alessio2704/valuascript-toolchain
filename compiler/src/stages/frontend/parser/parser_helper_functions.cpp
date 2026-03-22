#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
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
                Token key_token = cursor_.consume(TokenType::Identifier, key_err);
                cursor_.consume(TokenType::Colon, colon_err);
                auto val = parse_expression();

                if (is_expression_start(cursor_.peek().type) && cursor_.peek(1).type != TokenType::Colon) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                }

                return std::make_pair(key_token.lexeme, std::move(val));
            }
        );
    }
}
