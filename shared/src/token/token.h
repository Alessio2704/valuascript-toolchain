#pragma once
#include <string_view>
#include "token_type.h"
#include "source_span.h"
#include "comment_token.h"

namespace valuascript::shared {
    struct Token {
        TokenType type = TokenType::Error;
        std::string_view lexeme = {};
        size_t line = 0;
        size_t column = 0;
        size_t start_offset = 0;
        size_t length = 0;
    };
}

namespace valuascript::compiler {
    using Token = valuascript::shared::Token;
    using TokenType = valuascript::shared::TokenType;
    using CommentToken = valuascript::shared::CommentToken;
    using SourceSpan = valuascript::shared::SourceSpan;
}
