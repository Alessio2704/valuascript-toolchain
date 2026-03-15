#pragma once
#include <unordered_map>
#include <string>
#include <optional>

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
        At, // @
        Hash, // #
        Dot, // .

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
        PercentageLiteral, // 1%,

        // Keywords
        Import, // import
        Let, // let
        Var, // var
        If, // if
        Then, // then
        Else, // else
        True, // true
        False, // false
        And, // and
        Or, // or
        Not, // not
        Mod, // mod
        Func, // func
        Struct, // struct
        Enum, // enum
        Return, // return
        Switch, // switch
        Case, // case
        Default, // default
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

    inline std::string to_string(const TokenType type) {
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
            case TokenType::Hash: return "Hash";
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
            case TokenType::Var: return "Var";
            case TokenType::If: return "If";
            case TokenType::Then: return "Then";
            case TokenType::Else: return "Else";
            case TokenType::True: return "True";
            case TokenType::False: return "False";
            case TokenType::And: return "And";
            case TokenType::Or: return "Or";
            case TokenType::Not: return "Not";
            case TokenType::Mod: return "Mod";
            case TokenType::Func: return "Func";
            case TokenType::Return: return "Return";
            case TokenType::PercentageLiteral: return "PercentLiteral";
            case TokenType::Dot: return "Dot";
            case TokenType::Arrow: return "Arrow";
            case TokenType::Import: return "Import";
            case TokenType::Struct: return "Struct";
            case TokenType::Enum: return "Enum";
            case TokenType::Switch: return "Switch";
            case TokenType::Case: return "Case";
            case TokenType::Default: return "Default";
            case TokenType::EndOfFile: return "EndOfFile";
        }
        return "Unknown";
    }

    inline bool is_reserved_keyword(const TokenType type) {
        switch (type) {
            case TokenType::Import:
            case TokenType::Let:
            case TokenType::Var:
            case TokenType::Func:
            case TokenType::If:
            case TokenType::Then:
            case TokenType::Else:
            case TokenType::Return:
            case TokenType::Struct:
            case TokenType::Enum:
            case TokenType::True:
            case TokenType::False:
            case TokenType::And:
            case TokenType::Or:
            case TokenType::Not:
            case TokenType::Mod:
            case TokenType::Switch:
            case TokenType::Case:
            case TokenType::Default:
                return true;
            default:
                return false;
        }
    }

    inline std::optional<TokenType> get_keyword_type(const std::string &lexeme) {
        static const std::unordered_map<std::string, TokenType> kKeywords = {
            {"import", TokenType::Import},
            {"let", TokenType::Let},
            {"var", TokenType::Var},
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
            {"return", TokenType::Return},
        };

        if (const auto it = kKeywords.find(lexeme); it != kKeywords.end()) {
            return it->second;
        }
        return std::nullopt;
    }
}
