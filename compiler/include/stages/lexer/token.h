#pragma once

#pragma once
#include <string>

namespace valuascript::compiler {
    enum class TokenType {
        // Single-character punctuation & operators
        LeftParen, // (
        RightParen, // )
        LeftBracket, // [
        RightBracket, // ]
        LeftBrace, // {
        RightBrace, // }
        Comma, // ,
        Colon, // :
        At, // @ (for directives like @iterations)

        // Math operators
        Plus, // +
        Minus, // -
        Star, // *
        Slash, // /
        Caret, // ^

        // One or two character operators
        Assign, // =
        Equals, // ==
        NotEquals, // !=
        Greater, // >
        GreaterEqual, // >=
        Less, // <
        LessEqual, // <=
        Arrow, // ->

        // Literals
        Identifier, // CNAME (e.g., my_var, my_func)
        String, // "..."
        DocString, // """..."""
        Number, // 123, 12.3, 1_000 (SIGNED_NUMBER, FLOAT, INT)

        // Keywords
        Let, // let
        If, // if
        Then, // then
        Else, // else
        True, // true
        False, // false
        And, // and
        Or, // or
        Not, // not
        Func, // func
        Return, // return
        EndOfFile
    };

    struct Token {
        TokenType type;
        std::string lexeme;
        size_t line;
        size_t column;

        Token(const TokenType type, std::string lexeme, const size_t line, const size_t column)
            : type(type), lexeme(std::move(lexeme)), line(line), column(column) {
        }
    };

    inline std::string to_string(TokenType type) {
        switch (type) {
            case TokenType::LeftParen: return "LeftParen";
            case TokenType::RightParen: return "RightParen";
            case TokenType::LeftBracket: return "LeftBracket";
            case TokenType::RightBracket: return "RightBracket";
            case TokenType::LeftBrace: return "LeftBrace";
            case TokenType::RightBrace: return "RightBrace";
            case TokenType::Comma: return "Comma";
            case TokenType::Colon: return "Colon";
            case TokenType::At: return "At";
            case TokenType::Plus: return "Plus";
            case TokenType::Minus: return "Minus";
            case TokenType::Star: return "Star";
            case TokenType::Slash: return "Slash";
            case TokenType::Caret: return "Caret";
            case TokenType::Assign: return "Assign";
            case TokenType::Equals: return "Equals";
            case TokenType::NotEquals: return "NotEquals";
            case TokenType::Greater: return "Greater";
            case TokenType::GreaterEqual: return "GreaterEqual";
            case TokenType::Less: return "Less";
            case TokenType::LessEqual: return "LessEqual";
            case TokenType::Identifier: return "Identifier";
            case TokenType::String: return "String";
            case TokenType::DocString: return "DocString";
            case TokenType::Number: return "Number";
            case TokenType::Let: return "Let";
            case TokenType::If: return "If";
            case TokenType::Then: return "Then";
            case TokenType::Else: return "Else";
            case TokenType::True: return "True";
            case TokenType::False: return "False";
            case TokenType::And: return "And";
            case TokenType::Or: return "Or";
            case TokenType::Not: return "Not";
            case TokenType::Func: return "Func";
            case TokenType::Return: return "Return";
            case TokenType::EndOfFile: return "EndOfFile";
            default: return "Unknown";
        }
    }
}
