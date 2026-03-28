#pragma once

#include <memory>
#include <vector>
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/parser/token_cursor.h"

namespace valuascript::compiler {
    class Parser {
    private:
        TokenCursor cursor_;

    public:
        explicit Parser(TokenCursor cursor);

        std::unique_ptr<Program> parse_program();

    private:
        enum class Precedence {
            None = 0, Or = 1, And = 2, Comparison = 3, Term = 4, Factor = 5, Power = 6
        };

        static Precedence get_operator_precedence(TokenType type);

        [[nodiscard]] static bool is_operator_right_associative(TokenType type);

        const Token &consume_identifier(ValuascriptErrorCode fallback_err);

        void parse_top_level_declaration(Program *program);

        std::vector<Modifier> parse_modifiers();

        std::unique_ptr<ImportStatement> parse_import_statement();

        std::unique_ptr<Directive> parse_directive();

        std::unique_ptr<StructDefinition> parse_struct_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<EnumDefinition> parse_enum_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<FunctionDefinition> parse_function_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<TypeAnnotation> parse_type_annotation();

        std::unique_ptr<Statement> parse_function_body_statements();

        std::unique_ptr<Assignment> parse_assignment(std::vector<Modifier> modifiers);

        std::unique_ptr<Statement> parse_expression_statement();

        std::unique_ptr<ReturnStatement> parse_return_statement();

        std::unique_ptr<Expression> parse_expression(Precedence precedence = Precedence::Or);

        std::unique_ptr<Expression> parse_unary_expression();

        std::unique_ptr<Expression> parse_postfix_expression();

        std::unique_ptr<Expression> parse_primary_expression();

        std::unique_ptr<Expression> parse_tuple_or_grouping();

        std::unique_ptr<Expression> parse_tensor_literal();

        std::unique_ptr<Expression> parse_dict_literal();

        std::unique_ptr<Expression> parse_function_call(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_tensor_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_dot_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_conditional_expression();

        std::unique_ptr<Expression> parse_switch_expression();

        static bool is_valid_lvalue(const Expression *expr);

        static bool acts_like_identifier(const Token &token, TokenType next_type);

        static bool is_expression_start(TokenType type);

        static bool is_binary_operator(TokenType type);

        static bool is_unary_operator(TokenType type);

        void verify_statement_end() const;

        [[nodiscard]] static bool is_top_level_token(TokenType type);

        void synchronize();

        static bool is_structural_closer(TokenType type);

        void synchronize_to_closer(TokenType closing_token);

        std::vector<std::unique_ptr<Expression> > parse_expression_list(
            TokenType closing_token,
            std::optional<ValuascriptErrorCode> trailing_comma_err = std::nullopt,
            std::initializer_list<TokenType> panic_stops = {});

        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > parse_key_value_list(
            TokenType closing_token,
            ValuascriptErrorCode key_err,
            ValuascriptErrorCode colon_err,
            ValuascriptErrorCode missing_comma_err,
            std::optional<ValuascriptErrorCode> trailing_comma_err = std::nullopt,
            std::initializer_list<TokenType> panic_stops = {});

        template<typename T, typename IsElementStart, typename ElementParser>
            requires requires(IsElementStart is_start, ElementParser parser)
            {
                { is_start() } -> std::convertible_to<bool>;
                { parser() } -> std::convertible_to<T>;
            }
        std::vector<T> parse_comma_separated_list(
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const std::optional<ValuascriptErrorCode> missing_comma_err,
            const std::initializer_list<TokenType> &panic_stops,
            IsElementStart is_element_start,
            ElementParser parse_element) {
            std::vector<T> elements;

            auto is_hard_stop = [&](const Token &token, TokenType next_type) {
                if (acts_like_identifier(token, next_type)) return false;

                TokenType type = token.type;
                if (is_top_level_token(type)) return true;
                for (TokenType stop: panic_stops) {
                    if (type == stop) return true;
                }
                return false;
            };

            while (!cursor_.check(closing_token) && !cursor_.is_at_end()) {
                try {
                    elements.push_back(parse_element());

                    if (cursor_.match({TokenType::Comma})) {
                        if (cursor_.check(closing_token) && trailing_comma_err) {
                            cursor_.report_error(cursor_.previous(), *trailing_comma_err);
                        }
                    } else if (!cursor_.check(closing_token)) {
                        if (is_element_start()) {
                            if (missing_comma_err) {
                                try {
                                    cursor_.report_error(cursor_.peek(), *missing_comma_err);
                                } catch (const ParseSyncException &) {
                                }
                            }
                        } else {
                            break;
                        }
                    }
                } catch (const ParseSyncException &) {
                    while (!cursor_.is_at_end()) {
                        const Token &tok = cursor_.peek();
                        const TokenType next = cursor_.peek(1).type;

                        if (tok.type == TokenType::Comma || tok.type == closing_token) break;

                        if (is_top_level_token(tok.type) && !acts_like_identifier(tok, next)) {
                            break;
                        }

                        cursor_.advance();
                    }

                    if (cursor_.check(TokenType::Comma)) {
                        cursor_.advance();
                    }
                }

                const Token &current = cursor_.peek();
                if (current.type != closing_token && is_hard_stop(current, cursor_.peek(1).type)) {
                    break;
                }
            }
            return elements;
        }

        template<typename T, typename ElementParser>
            requires requires(ElementParser parser)
            {
                { parser() } -> std::convertible_to<T>;
            }
        std::vector<T> parse_comma_separated_list(
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const ValuascriptErrorCode missing_comma_err,
            const std::initializer_list<TokenType> &panic_stops,
            ElementParser parse_element) {
            return parse_comma_separated_list<T>(
                closing_token,
                trailing_comma_err,
                missing_comma_err,
                panic_stops,
                [this]() {
                    const Token &tok = cursor_.peek();
                    return tok.type == TokenType::Identifier || acts_like_identifier(tok, cursor_.peek(1).type) || tok.
                           type == TokenType::LeftParen;
                },
                parse_element
            );
        }
    };
}
