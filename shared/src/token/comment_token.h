#pragma once
#include <string>
#include <cstddef>
#include "source_span.h"

namespace valuascript::shared
{
    struct CommentToken
    {
        std::string text = {};
        size_t line = 0;
        size_t column = 0;
        size_t start_offset = 0;
        size_t length = 0;
        SourceSpan span = {};
    };
}

namespace valuascript::compiler
{
    using CommentToken = valuascript::shared::CommentToken;
}
