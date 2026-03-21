#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    Parser::Precedence Parser::get_operator_precedence(const TokenType type) {
        switch (type) {
            case TokenType::Or:
                return Precedence::Or;
            case TokenType::And:
                return Precedence::And;
            case TokenType::Equals:
            case TokenType::NotEquals:
            case TokenType::Less:
            case TokenType::LessEqual:
            case TokenType::Greater:
            case TokenType::GreaterEqual:
                return Precedence::Comparison;
            case TokenType::Plus:
            case TokenType::Minus:
                return Precedence::Term;
            case TokenType::Star:
            case TokenType::Slash:
            case TokenType::Mod:
                return Precedence::Factor;
            case TokenType::Caret:
                return Precedence::Power;
            default:
                return Precedence::None;
        }
    }

    bool Parser::is_operator_right_associative(TokenType type) {
        switch (type) {
            case TokenType::Caret:
                return true;
            default:
                return false;
        }
    }

    bool Parser::is_binary_operator(const TokenType type) {
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

    bool Parser::is_unary_operator(const TokenType type) {
        switch (type) {
            case TokenType::Plus:
            case TokenType::Minus:
            case TokenType::Not:
                return true;
            default:
                return false;
        }
    }
}
