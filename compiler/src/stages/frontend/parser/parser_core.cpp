#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    Parser::Parser(TokenCursor cursor) : cursor_(std::move(cursor)) {
    }

    std::unique_ptr<Program> Parser::parse_program() {
        auto program = std::make_unique<Program>();
        const Token &start_token = cursor_.peek();

        while (!cursor_.is_at_end()){
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

    void Parser::synchronize() {

        while (!cursor_.is_at_end()) {
            switch (cursor_.peek().type) {
                case TokenType::Let:
                case TokenType::Var:
                case TokenType::Func:
                case TokenType::Struct:
                case TokenType::Enum:
                case TokenType::Import:
                case TokenType::Return:
                case TokenType::If:
                    return;
                default:
                    break;
            }

            cursor_.advance();
        }
    }
}
