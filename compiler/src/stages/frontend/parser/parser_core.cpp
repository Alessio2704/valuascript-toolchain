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
                        cursor_.report_error(cursor_.peek(), ErrorCode::UnexpectedToken,
                                             "Syntax Error: Invalid syntax. Expected '#', 'let', 'var', 'enum', 'struct', 'func' or an identifier.");
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

    void Parser::check_trailing_expression() const {
        if (!cursor_.is_at_end() && cursor_.peek().line == cursor_.previous().line) {
            if (is_expression_start(cursor_.peek().type)) {
                cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator,
                                     "Syntax Error: Missing operator between expressions.");
            }
        }
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
