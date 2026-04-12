#pragma once
#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include "token.h"

namespace valuascript::shared {
    inline const std::unordered_map<std::string, TokenType> kReservedKeywords = {
        {"import", TokenType::Import},
        {"let", TokenType::Let},
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
        {"typealias", TokenType::Typealias},
        {"return", TokenType::Return},
    };

    std::optional<TokenType> get_keyword_type(const std::string &lexeme);

    bool is_reserved_keyword(const Token &token);

    std::vector<std::string> get_all_reserved_keyword_strings();
}
