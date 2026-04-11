#pragma once
#include <unordered_map>
#include <optional>
#include <string>
#include "token.h"

namespace valuascript::shared {
    inline const std::unordered_map<std::string, TokenType> kReservedKeywords = {
        {"import", TokenType::Import},
        {"let", TokenType::Let},
        {"var", TokenType::Var},
        {"if", TokenType::If},
        {"then", TokenType::Then},
        {"else", TokenType::Else},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"and", TokenType::And},
        {"or", TokenType::Or},
        {"not", TokenType::Not},
        {"mod", TokenType::Mod},
        {"func", TokenType::Func},
        {"struct", TokenType::Struct},
        {"enum", TokenType::Enum},
        {"switch", TokenType::Switch},
        {"case", TokenType::Case},
        {"default", TokenType::Default},
        {"self", TokenType::Self},
        {"return", TokenType::Return},
    };

    inline std::optional<TokenType> get_keyword_type(const std::string &lexeme) {
        if (const auto it = kReservedKeywords.find(lexeme); it != kReservedKeywords.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    inline bool is_reserved_keyword(const Token &token) {
        if (const auto res = get_keyword_type(token.lexeme); res.has_value()) {
            return true;
        }
        return false;
    }
}