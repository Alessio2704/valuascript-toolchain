#pragma once
#include <string>
#include "token_type.h"

namespace valuascript::shared {
    struct Token {
        TokenType type;
        std::string lexeme;
        size_t line;
        size_t column;

        Token(const TokenType t, std::string lex, const size_t l, const size_t c)
            : type(t), lexeme(std::move(lex)), line(l), column(c) {
        }
    };
}
