#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/parser/token_cursor.h"
#include "stages/frontend/parser/token_traits.h"

namespace valuascript::compiler {
    class Parser {
    private:
        struct CloserTracker {
            Parser &parser;

            CloserTracker(Parser &p, TokenType t) : parser(p) {
                parser.active_closers_.push_back(t);
            }

            ~CloserTracker() {
                parser.active_closers_.pop_back();
            }
        };

        TokenCursor cursor_;
        std::vector<TokenType> active_closers_;

    public:
        explicit Parser(TokenCursor cursor);

        std::unique_ptr<Program> parse_program();

    private:
        enum class ParseContext { TopLevel, FunctionBody };

        void parse_statement_or_declaration(ParseContext ctx, Program *program,
                                            std::vector<std::unique_ptr<Statement> > &block);

        std::unique_ptr<ImportStatement> parse_import_statement();

        std::unique_ptr<Directive> parse_directive();

        std::vector<Modifier> parse_modifiers(bool is_statement_context = false);

        std::unique_ptr<StructDefinition> parse_struct_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<EnumDefinition> parse_enum_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<TypeAliasDefinition> parse_type_alias_definition(std::vector<Modifier> modifiers);

        std::unique_ptr<FunctionDefinition> parse_function_definition(std::vector<Modifier> modifiers);

        /**
        * A predicate used during nested list parsing (like generic arguments or tuple types).
        * It helps identify if we have "leaked" out of the current type and are looking at
        * tokens that actually belong to the next element in the caller's list
        * (e.g. the next field in a struct or the next parameter in a function).
         */
        using ParentBoundaryPredicate = std::function<bool(int lookahead)>;

        std::unique_ptr<TypeAnnotation> parse_type_annotation(ParentBoundaryPredicate is_at_parent_boundary = nullptr);

        std::unique_ptr<Assignment> parse_assignment(std::vector<Modifier> modifiers);

        std::unique_ptr<Statement> parse_expression_statement();

        std::unique_ptr<ReturnStatement> parse_return_statement();

        std::unique_ptr<Expression> parse_expression(Precedence min_precedence = Precedence::Or);

        std::unique_ptr<Expression> parse_unary_expression();

        std::unique_ptr<Expression> parse_postfix_expression();

        std::unique_ptr<Expression> parse_primary_expression();

        std::unique_ptr<Expression> parse_tuple_or_grouping();

        std::unique_ptr<Expression> complete_tuple(std::unique_ptr<Expression> first_expr, const Token &start_token);

        std::unique_ptr<Expression> complete_grouping(std::unique_ptr<Expression> first_expr, bool first_expr_failed,
                                                      const Token &start_token);

        std::unique_ptr<Expression> parse_tensor_literal();

        std::unique_ptr<Expression> parse_dict_literal();

        std::unique_ptr<Expression> parse_function_call(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_tensor_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_dot_access(std::unique_ptr<Expression> target);

        std::unique_ptr<Expression> parse_conditional_expression();

        std::unique_ptr<Expression> parse_switch_expression();

        std::unique_ptr<Expression> parse_switch_target();

        void parse_switch_body(std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > &cases,
                               std::unique_ptr<Expression> &default_case);

        std::pair<std::vector<std::string>, std::unique_ptr<Expression> > parse_switch_case();

        std::unique_ptr<Expression> parse_switch_default();

        std::unique_ptr<Expression> parse_switch_result();

        const Token &consume_identifier(ValuascriptErrorCode fallback_err, bool allow_top_level_keywords = true,
                                        bool check_statement_boundary = false);

        void verify_statement_end() const;

        void consume_unexpected_statement_gracefully();

        [[nodiscard]] TokenType peek_past_modifiers() const;

        [[nodiscard]] bool is_at_top_level_declaration() const;

        [[nodiscard]] bool is_at_any_declaration() const;

        [[nodiscard]] bool is_missing_closing_brace() const;

        [[nodiscard]] bool is_active_closer(TokenType type) const;

        using SyncPredicate = std::function<bool(TokenType type, int nesting_depth)>;

        void recover(const SyncPredicate &stop_condition);

        void synchronize();

        void synchronize_block_statement();

        void synchronize_to_closer(TokenType closing_token);

        void synchronize_and_consume_closer(TokenType expected_closer);

        void synchronize_to_switch_boundary();

        void synchronize_to_conditional_boundary();

        std::vector<std::unique_ptr<Expression> > parse_expression_list(
            TokenType closing_token,
            std::optional<ValuascriptErrorCode> trailing_comma_err = std::nullopt,
            std::vector<TokenType> recovery_boundaries = {});

        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > parse_key_value_list(
            TokenType closing_token,
            ValuascriptErrorCode key_err,
            ValuascriptErrorCode colon_err,
            ValuascriptErrorCode missing_comma_err,
            std::optional<ValuascriptErrorCode> trailing_comma_err = std::nullopt,
            std::vector<TokenType> recovery_boundaries = {});

        template<typename ElementType, typename IsElementStart, typename ElementParser>
            requires requires(IsElementStart is_start, ElementParser parser)
            {
                { is_start() } -> std::convertible_to<bool>;
                { parser() } -> std::convertible_to<ElementType>;
            }
        std::vector<ElementType> parse_list(
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const std::optional<ValuascriptErrorCode> missing_comma_err,
            const std::vector<TokenType> &recovery_boundaries,
            IsElementStart is_element_start,
            ElementParser parse_element,
            const ParentBoundaryPredicate &is_at_parent_boundary = nullptr) {
            std::vector<ElementType> elements;

            auto is_hard_stop = [&](const Token &token, TokenType next) {
                if (is_element_start()) {
                    if (TokenTraits::is_newline_statement_boundary(cursor_.previous(), token, next)) {
                        if (token.type != TokenType::At || is_at_any_declaration()) {
                            return true;
                        }
                    }
                    return false;
                }

                if (TokenTraits::is_newline_statement_boundary(cursor_.previous(), token, next)) return true;
                for (TokenType stop: recovery_boundaries) if (token.type == stop) return true;
                return false;
            };

            while (!cursor_.check(closing_token) && !cursor_.is_at_end()) {
                if (is_at_parent_boundary && is_at_parent_boundary(0)) break;

                try {
                    const Token &tok = cursor_.peek();
                    TokenType next = cursor_.peek(1).type;

                    if (TokenTraits::is_statement_start(tok, next) && !is_element_start()) {
                        if (tok.line > cursor_.previous().line) {
                            break;
                        } else {
                            const Token &start_tok = cursor_.peek();
                            consume_unexpected_statement_gracefully();
                            SourceSpan span = cursor_.make_span(start_tok, cursor_.previous());
                            cursor_.report_error_no_panic(
                                span, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
                            throw ParseSyncException();
                        }
                    }

                    elements.push_back(parse_element());

                    if (is_at_parent_boundary && is_at_parent_boundary(0)) {
                        break;
                    }

                    if (cursor_.check(TokenType::Comma)) {
                        if (is_at_parent_boundary && is_at_parent_boundary(1)) {
                            break;
                        }
                        cursor_.advance();

                        if (cursor_.check(closing_token) && trailing_comma_err) {
                            cursor_.report_error(cursor_.previous(), *trailing_comma_err);
                        }
                    } else if (!cursor_.check(closing_token)) {
                        bool is_boundary = TokenTraits::is_newline_statement_boundary(
                            cursor_.previous(), cursor_.peek(), cursor_.peek(1).type);

                        if (is_boundary && cursor_.peek().type == TokenType::At) {
                            if (is_at_any_declaration()) {
                            } else {
                                is_boundary = false;
                            }
                        }

                        if (is_element_start() && !is_boundary) {
                            if (missing_comma_err) {
                                cursor_.report_error_no_panic(cursor_.peek(), *missing_comma_err);
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

                        if (tok.type != TokenType::At && TokenTraits::is_newline_statement_boundary(
                                cursor_.previous(), tok, next))
                            break;
                        if (is_at_parent_boundary && is_at_parent_boundary(0)) break;

                        cursor_.advance();
                    }

                    if (cursor_.check(TokenType::Comma)) {
                        if (is_at_parent_boundary && is_at_parent_boundary(1)) {
                            break;
                        }
                        cursor_.advance();
                    }
                }

                if (cursor_.peek().type != closing_token && is_hard_stop(cursor_.peek(), cursor_.peek(1).type)) break;
            }
            return elements;
        }

        template<typename ElementType, typename ElementParser>
            requires requires(ElementParser parser)
            {
                { parser() } -> std::convertible_to<ElementType>;
            }
        std::vector<ElementType> parse_list(
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const ValuascriptErrorCode missing_comma_err,
            const std::vector<TokenType> &recovery_boundaries,
            ElementParser parse_element,
            ParentBoundaryPredicate is_at_parent_boundary = nullptr) {
            return parse_list<ElementType>(
                closing_token,
                trailing_comma_err,
                std::make_optional(missing_comma_err),
                recovery_boundaries,
                [this]() {
                    const Token &tok = cursor_.peek();
                    return tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(
                               tok, cursor_.peek(1).type) || tok.type == TokenType::LeftParen;
                },
                parse_element,
                is_at_parent_boundary
            );
        }
    };
}
