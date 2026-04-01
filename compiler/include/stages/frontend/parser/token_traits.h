#pragma once
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler {
    enum class Precedence {
        None = 0, Or = 1, And = 2, Comparison = 3, Term = 4, Factor = 5, Power = 6
    };

    struct TokenTraits {
        static Precedence get_operator_precedence(TokenType type);

        static bool is_operator_right_associative(TokenType type);

        static bool is_top_level_only_declaration(TokenType type);

        static bool is_valid_lvalue(const Expression *expr);

        static bool acts_like_identifier(const Token &token, TokenType next_type);

        static bool is_expression_start(TokenType type);

        static bool is_binary_operator(TokenType type);

        static bool is_unary_operator(TokenType type);

        static bool is_dangling_operator(TokenType type);

        static bool is_top_level_token(TokenType type);

        static bool is_statement_start(const Token &token, TokenType next_type);

        static bool is_identifier_start(const Token &token);

        static bool is_grouping_opener(TokenType type);

        static bool is_grouping_closer(TokenType type);
    };
}
