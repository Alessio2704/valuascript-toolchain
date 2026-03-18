#pragma once
#include <string>
#include "token_type.h"

namespace valuascript::shared {
    struct Token {
        TokenType type;
        std::string lexeme;
        size_t line;
        size_t column;

        Token(const TokenType type, std::string lexeme, const size_t line, const size_t column)
            : type(type), lexeme(std::move(lexeme)), line(line), column(column) {
        }
    };
}
