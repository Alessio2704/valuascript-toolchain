#include "lexer_error_code.h"

namespace valuascript::compiler
{
    std::string_view get_error_template(LexerErrorCode code)
    {
        switch (code)
        {
        case LexerErrorCode::InvalidCharacter:
            return "Syntax Error: Invalid character '{}' found.";
        case LexerErrorCode::UnclosedString:
            return "Syntax Error: Unclosed string literal.";
        case LexerErrorCode::InvalidIdentifier:
            return "Syntax Error: Invalid identifier name.";
        case LexerErrorCode::UnterminatedDecimal:
            return "Syntax Error: Unterminated decimal number. Expected digits after '.'.";
        case LexerErrorCode::TrailingSeparatorInNumberLiteral:
            return
                "Syntax Error: Trailing separator '_' in number literal (remove it or complete with digits after separator).";
        }
        return "Unknown Lexical Error";
    }
}
