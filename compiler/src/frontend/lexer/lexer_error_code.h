#pragma once
#include <string_view>

namespace valuascript::compiler
{
    enum class LexerErrorCode
    {
        InvalidCharacter = 2001,
        UnclosedString,
        InvalidIdentifier,
        UnterminatedDecimal,
        TrailingSeparatorInNumberLiteral
    };

    std::string_view get_error_template(LexerErrorCode code);
}
