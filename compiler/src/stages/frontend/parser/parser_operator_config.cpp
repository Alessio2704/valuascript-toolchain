#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    Precedence TokenTraits::get_operator_precedence(const TokenType op_type) {
        switch (op_type) {
            case TokenType::Or: return Precedence::Or;
            case TokenType::And: return Precedence::And;
            case TokenType::Equals:
            case TokenType::NotEquals:
            case TokenType::Less:
            case TokenType::LessEqual:
            case TokenType::Greater:
            case TokenType::GreaterEqual: return Precedence::Comparison;
            case TokenType::Plus:
            case TokenType::Minus: return Precedence::Term;
            case TokenType::Star:
            case TokenType::Slash:
            case TokenType::Mod: return Precedence::Factor;
            case TokenType::Caret: return Precedence::Power;
            default: return Precedence::None;
        }
    }

    bool TokenTraits::is_operator_right_associative(TokenType op_type) {
        return op_type == TokenType::Caret;
    }

    bool TokenTraits::is_binary_operator(const TokenType type) {
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
            case TokenType::Or: return true;
            default: return false;
        }
    }

    bool TokenTraits::is_unary_operator(const TokenType type) {
        return type == TokenType::Plus || type == TokenType::Minus || type == TokenType::Not;
    }

    bool TokenTraits::is_dangling_operator(const TokenType type) {
        return is_binary_operator(type) || is_unary_operator(type) ||
               type == TokenType::Assign || type == TokenType::Return || type == TokenType::Comma ||
               type == TokenType::Colon || type == TokenType::Arrow || type == TokenType::Then || type ==
               TokenType::Else;
    }
}
