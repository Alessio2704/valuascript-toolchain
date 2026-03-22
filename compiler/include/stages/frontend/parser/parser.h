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

        static bool is_expression_start(TokenType type);

        static bool is_binary_operator(TokenType type);

        static bool is_unary_operator(TokenType type);

        void verify_statement_end() const;

        [[nodiscard]] static bool is_top_level_token(TokenType type);

        void synchronize();

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

        template<typename T, typename PreCheck, typename RecoveryCheck, typename ElementParser, typename
            MissingCommaHandler>
            requires requires(PreCheck pre_check, RecoveryCheck rec_check, ElementParser parser,
                              MissingCommaHandler handler)
            {
                { pre_check() } -> std::convertible_to<bool>;
                { rec_check() } -> std::convertible_to<bool>;
                { parser() } -> std::convertible_to<T>;
                { handler() };
            }
        std::vector<T> parse_comma_separated_list(
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const std::initializer_list<TokenType> &panic_stops,
            PreCheck pre_check,
            RecoveryCheck recovery_check,
            ElementParser parse_element,
            MissingCommaHandler missing_comma_handler) {
            std::vector<T> elements;

            while (!cursor_.check(closing_token) && !cursor_.is_at_end()) {
                if (!pre_check()) break;

                try {
                    elements.push_back(parse_element());

                    if (cursor_.match({TokenType::Comma})) {
                        if (cursor_.check(closing_token) && trailing_comma_err) {
                            cursor_.report_error(cursor_.previous(), *trailing_comma_err);
                        }
                    } else if (!cursor_.check(closing_token)) {
                        bool missing_comma_reported = false;
                        try {
                            missing_comma_handler();
                        } catch (const ParseSyncException &) {
                            missing_comma_reported = true;
                        }

                        if (!missing_comma_reported) {
                            break;
                        }

                        if (!recovery_check()) {
                            throw ParseSyncException();
                        }
                    }
                } catch (const ParseSyncException &) {
                    while (!cursor_.is_at_end()) {
                        TokenType next = cursor_.peek().type;
                        if (next == closing_token || next == TokenType::Comma) break;
                        if (is_top_level_token(next)) break;

                        bool hit_panic_stop = false;
                        for (TokenType stop: panic_stops) {
                            if (next == stop) {
                                hit_panic_stop = true;
                                break;
                            }
                        }
                        if (hit_panic_stop) break;

                        cursor_.advance();
                    }
                    if (cursor_.check(TokenType::Comma)) {
                        cursor_.advance();
                    }
                }

                TokenType current = cursor_.peek().type;
                if (is_top_level_token(current) && current != closing_token) break;

                bool hit_panic_stop = false;
                for (TokenType stop: panic_stops) {
                    if (current == stop) {
                        hit_panic_stop = true;
                        break;
                    }
                }
                if (hit_panic_stop && current != closing_token) break;
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
                panic_stops,
                []() { return true; },
                [this]() { return cursor_.check(TokenType::Identifier) || cursor_.check(TokenType::LeftParen); },
                parse_element, [this, missing_comma_err]() {
                    if (cursor_.check(TokenType::Identifier)) {
                        cursor_.report_error(cursor_.peek(), missing_comma_err);
                    }
                }
            );
        }
    };
}
