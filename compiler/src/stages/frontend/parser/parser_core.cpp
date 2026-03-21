#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    Parser::Parser(TokenCursor cursor) : cursor_(std::move(cursor)) {
    }

    std::unique_ptr<Program> Parser::parse_program() {
        auto program = std::make_unique<Program>();
        const Token &start_token = cursor_.peek();

        while (!cursor_.is_at_end()) {
            try {
                switch (cursor_.peek().type) {
                    case TokenType::Import:
                        program->import_statements.push_back(parse_import_statement());
                        break;
                    case TokenType::Hash:
                        program->directives.push_back(parse_directive());
                        break;
                    case TokenType::At:
                    case TokenType::Let:
                    case TokenType::Var:
                    case TokenType::Func:
                    case TokenType::Struct:
                    case TokenType::Enum:
                        parse_top_level_declaration(program.get());
                        break;
                    case TokenType::Identifier:
                        program->execution_steps.push_back(parse_expression_statement());
                        break;
                    default:
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::UnexpectedTopLevelToken);
                }
            } catch (const ParseSyncException &) {
                synchronize();
            }
        }

        program->span = cursor_.make_span(start_token, cursor_.previous());
        return program;
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
        const std::optional<ValuascriptErrorCode> trailing_comma_err) {
        return parse_comma_separated_list<std::unique_ptr<Expression> >(
            closing_token,
            trailing_comma_err,
            [this]() { return is_expression_start(cursor_.peek().type); },
            [this]() { return parse_expression(); },
            [this]() {
                if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(),
                                         ValuascriptErrorCode::MissingCommaOrOperatorBetweenExpressions);
                }
            }
        );
    }

    std::vector<std::pair<std::string, std::unique_ptr<Expression> > > Parser::parse_key_value_list(
        const TokenType closing_token,
        const ValuascriptErrorCode key_err,
        const ValuascriptErrorCode colon_err,
        const ValuascriptErrorCode missing_comma_err,
        const std::optional<ValuascriptErrorCode> trailing_comma_err) {
        return parse_comma_separated_list<std::pair<std::string, std::unique_ptr<Expression> > >(
            closing_token,
            trailing_comma_err,
            []() { return true; },
            [this, key_err, colon_err]() {
                Token key_token = cursor_.consume(TokenType::Identifier, key_err);
                cursor_.consume(TokenType::Colon, colon_err);
                return std::make_pair(key_token.lexeme, parse_expression());
            },
            [this, missing_comma_err]() {
                if (cursor_.check(TokenType::Identifier) && cursor_.peek(1).type == TokenType::Colon) {
                    cursor_.report_error(cursor_.peek(), missing_comma_err);
                } else if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                }
            }
        );
    }

    void Parser::synchronize() {
        while (!cursor_.is_at_end()) {
            if (is_top_level_token(cursor_.peek().type)) {
                return;
            }
            cursor_.advance();
        }
    }
}
