#pragma once
#include <string_view>
#include "token_type.h"

namespace valuascript::shared {
    struct Token {
        TokenType type;
        std::string_view lexeme;
        size_t line;
        size_t column;

        Token(const TokenType t, std::string_view lex, const size_t l, const size_t c)
            : type(t), lexeme(lex), line(l), column(c) {
        }
    };
}
