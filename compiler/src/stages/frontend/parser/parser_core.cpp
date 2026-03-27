#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    Parser::Parser(TokenCursor cursor) : cursor_(std::move(cursor)) {
    }

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

    void Parser::synchronize_to_closer(TokenType closing_token) {
        int internal_depth = 0;
        while (!cursor_.is_at_end()) {
            const Token &tok = cursor_.peek();

            if (internal_depth == 0) {
                if (tok.type == closing_token) {
                    break;
                }

                if (is_top_level_token(tok.type)) {
                    break;
                }
            }

            if (tok.type == TokenType::LeftParen ||
                tok.type == TokenType::LeftBracket ||
                tok.type == TokenType::LeftBrace ||
                tok.type == TokenType::Less) {
                internal_depth++;
                } else if (tok.type == TokenType::RightParen ||
                           tok.type == TokenType::RightBracket ||
                           tok.type == TokenType::RightBrace ||
                           tok.type == TokenType::Greater) {
                    internal_depth--;
                           }

            cursor_.advance();
            if (internal_depth < 0) internal_depth = 0;
        }
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
