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
                        cursor_.report_error(cursor_.peek(), ErrorCode::UnexpectedTopLevelToken);
                }
            } catch (const ParseSyncException &) {
                synchronize();
            }
        }

        program->span = cursor_.make_span(start_token, cursor_.previous());
        return program;
    }

    bool Parser::is_valid_lvalue(const Expression *expr) {
        if (dynamic_cast<const IdentifierAccess *>(expr) != nullptr) return true;
        if (dynamic_cast<const DotAccess *>(expr) != nullptr) return true;
        if (dynamic_cast<const BracketAccess *>(expr) != nullptr) return true;
        return false;
    }

    bool Parser::is_expression_start(const TokenType type) {
        switch (type) {
            case TokenType::Number:
            case TokenType::PercentageLiteral:
            case TokenType::String:
            case TokenType::DocString:
            case TokenType::True:
            case TokenType::False:
            case TokenType::Identifier:
            case TokenType::Switch:
            case TokenType::If:
            case TokenType::LeftParen:
            case TokenType::LeftBracket:
            case TokenType::LeftBrace:
            case TokenType::Minus:
            case TokenType::Plus:
            case TokenType::Not:
                return true;
            default:
                return false;
        }
    }

    bool Parser::is_binary_operator(TokenType type) {
        switch (type) {
            case TokenType::Plus:
            case TokenType::Minus:
            case TokenType::Star:
            case TokenType::Slash:
            case TokenType::Mod:
            case TokenType::Caret:
            case TokenType::Equals:
            case TokenType::NotEquals:
            case TokenType::Less:
            case TokenType::LessEqual:
            case TokenType::Greater:
            case TokenType::GreaterEqual:
            case TokenType::And:
            case TokenType::Or:
                return true;
            default:
                return false;
        }
    }

    void Parser::verify_statement_end() const {
        if (!cursor_.is_at_end() && cursor_.peek().line == cursor_.previous().line) {
            if (is_expression_start(cursor_.peek().type)) {
                cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator);
            }
        }
    }

    std::vector<std::unique_ptr<Expression> > Parser::parse_expression_list(
        TokenType closing_token, std::optional<ErrorCode> trailing_comma_err) {
        std::vector<std::unique_ptr<Expression> > elements;
        while (!cursor_.check(closing_token) && !cursor_.is_at_end()) {
            if (!is_expression_start(cursor_.peek().type)) break;

            elements.push_back(parse_expression());
            if (cursor_.match({TokenType::Comma})) {
                if (cursor_.check(closing_token) && trailing_comma_err) {
                    cursor_.report_error(cursor_.previous(), *trailing_comma_err);
                }
            } else if (!cursor_.check(closing_token)) {
                if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(), ErrorCode::MissingCommaOrOperatorBetweenExpressions);
                } else {
                    break;
                }
            }
        }
        return elements;
    }

    std::vector<std::pair<std::string, std::unique_ptr<Expression> > > Parser::parse_key_value_list(
        TokenType closing_token, ErrorCode key_err, ErrorCode colon_err,
        ErrorCode missing_comma_err, std::optional<ErrorCode> trailing_comma_err) {
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > pairs;
        while (!cursor_.check(closing_token) && !cursor_.is_at_end()) {
            Token key_token = cursor_.consume(TokenType::Identifier, key_err);
            cursor_.consume(TokenType::Colon, colon_err);

            pairs.emplace_back(key_token.lexeme, parse_expression());

            if (cursor_.match({TokenType::Comma})) {
                if (cursor_.check(closing_token) && trailing_comma_err) {
                    cursor_.report_error(cursor_.previous(), *trailing_comma_err);
                }
            } else if (cursor_.check(TokenType::Identifier) && cursor_.peek(1).type == TokenType::Colon) {
                cursor_.report_error(cursor_.peek(), missing_comma_err);
            } else if (!cursor_.check(closing_token) && is_expression_start(cursor_.peek().type)) {
                cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator);
            } else {
                break;
            }
        }
        return pairs;
    }

    void Parser::synchronize() {
        while (!cursor_.is_at_end()) {
            switch (cursor_.peek().type) {
                case TokenType::Let:
                case TokenType::Var:
                case TokenType::Func:
                case TokenType::Struct:
                case TokenType::Enum:
                case TokenType::Import:
                case TokenType::At:
                case TokenType::Hash:
                    return;
                default:
                    break;
            }
            cursor_.advance();
        }
    }
}
