#pragma once
#include <string_view>
#include "token_type.h"

namespace valuascript::shared {
    struct Token {
        TokenType type = TokenType::Error;
        std::string_view lexeme = {};
        size_t line = 0;
        size_t column = 0;
    };
}
