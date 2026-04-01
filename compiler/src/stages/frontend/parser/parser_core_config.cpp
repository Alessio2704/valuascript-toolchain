#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    bool TokenTraits::is_valid_lvalue(const Expression *expr) {
        return dynamic_cast<const IdentifierAccess *>(expr) ||
               dynamic_cast<const DotAccess *>(expr) ||
               dynamic_cast<const BracketAccess *>(expr);
    }

    bool TokenTraits::is_expression_start(const TokenType type) {
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
            case TokenType::Not: return true;
            default: return false;
        }
    }

    bool TokenTraits::is_statement_start(const Token &token, TokenType next_type) {
        return is_top_level_token(token.type) && !acts_like_identifier(token, next_type);
    }

    bool TokenTraits::is_identifier_start(const Token &token) {
        return token.type == TokenType::Identifier || (
                   is_reserved_keyword(token) && !is_top_level_only_declaration(token.type));
    }

    bool TokenTraits::acts_like_identifier(const Token &token, TokenType next_type) {
        if (!is_reserved_keyword(token)) return false;
        return (next_type == TokenType::Comma || next_type == TokenType::Colon || next_type == TokenType::Assign ||
                next_type == TokenType::LeftParen || next_type == TokenType::RightParen || next_type ==
                TokenType::LeftBrace ||
                next_type == TokenType::RightBrace || next_type == TokenType::Less || next_type == TokenType::Greater ||
                next_type == TokenType::EndOfFile);
    }

    bool TokenTraits::is_top_level_token(const TokenType type) {
        switch (type) {
            case TokenType::Let:
            case TokenType::Var:
            case TokenType::Func:
            case TokenType::Struct:
            case TokenType::Enum:
            case TokenType::Import:
            case TokenType::At:
            case TokenType::Hash: return true;
            default: return false;
        }
    }

    bool TokenTraits::is_grouping_opener(const TokenType type) {
        return type == TokenType::LeftParen || type == TokenType::LeftBracket || type == TokenType::LeftBrace;
    }

    bool TokenTraits::is_grouping_closer(const TokenType type) {
        return type == TokenType::RightParen || type == TokenType::RightBracket || type == TokenType::RightBrace;
    }

    bool TokenTraits::is_top_level_only_declaration(TokenType type) {
        switch (type) {
            case TokenType::Import:
            case TokenType::Hash:
            case TokenType::Func:
            case TokenType::Struct:
            case TokenType::Enum: return true;
            default: return false;
        }
    }
}
