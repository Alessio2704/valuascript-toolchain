#pragma once

namespace valuascript::shared  {
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
}
