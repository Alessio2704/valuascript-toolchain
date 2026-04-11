#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"
#include <algorithm>

namespace valuascript::compiler {
    std::unique_ptr<Expression> Parser::parse_expression(const Precedence min_precedence) {
        auto left = parse_unary_expression();

        while (TokenTraits::get_operator_precedence(cursor_.peek().type) >= min_precedence) {
            bool inside_expr_grouping = std::any_of(active_closers_.begin(), active_closers_.end(), [](TokenType t) {
                return t == TokenType::RightParen || t == TokenType::RightBracket;
            });

            if (cursor_.peek().line > cursor_.previous().line && !inside_expr_grouping) {
                break;
            }

            Token op = cursor_.advance();

            Precedence op_precedence = TokenTraits::get_operator_precedence(op.type);

            const Precedence next_precedence = TokenTraits::is_operator_right_associative(op.type)
                                                   ? op_precedence
                                                   : static_cast<Precedence>(static_cast<int>(op_precedence) + 1);

            auto right = parse_expression(next_precedence);

            const SourceSpan combined = cursor_.combine_spans(left->span, right->span);

            left = std::make_unique<BinaryExpression>(std::move(left), op.type, std::move(right));

            left->span = combined;

            if (op_precedence == Precedence::Comparison &&
                TokenTraits::get_operator_precedence(cursor_.peek().type) == Precedence::Comparison) {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations);
            }
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parse_unary_expression() {
        if (TokenTraits::is_unary_operator(cursor_.peek().type)) {
            Token op = cursor_.advance();
            auto right = parse_unary_expression();
            auto unary = std::make_unique<UnaryExpression>(op.type, std::move(right));
            unary->span = cursor_.make_span(op, cursor_.previous());
            return unary;
        }
        return parse_postfix_expression();
    }

    std::unique_ptr<Expression> Parser::parse_postfix_expression() {
        auto expr = parse_primary_expression();
        while (true) {
            SourceSpan start_span = expr->span;
            if (cursor_.match({TokenType::LeftParen})) {
                expr = parse_function_call(std::move(expr));
            } else if (cursor_.match({TokenType::LeftBracket})) {
                expr = parse_tensor_access(std::move(expr));
            } else if (cursor_.match({TokenType::Dot})) {
                expr = parse_dot_access(std::move(expr));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_function_call(std::unique_ptr<Expression> target) {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(*this, TokenType::RightParen);

        if (!cursor_.check(TokenType::RightParen) && !cursor_.is_at_end()) {
            const Token &p0 = cursor_.peek();
            const TokenType p1 = cursor_.peek(1).type;

            bool is_id_like = p0.type == TokenType::Identifier || TokenTraits::acts_like_identifier(p0, p1);

            if (is_id_like) {
                if (p1 != TokenType::Colon && TokenTraits::is_binary_operator(p1)) {
                    cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MissingOperatorOrArgumentName);
                }
            } else {
                if (TokenTraits::is_expression_start(p0.type)) {
                    cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MissingOperatorOrArgumentName);
                } else if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::Comma)) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ExpectedArgumentNameOrClosingParen);
                }
            }
        }

        auto arguments = parse_key_value_list(
            TokenType::RightParen,
            ValuascriptErrorCode::MissingArgumentNameInFunctionCall,
            ValuascriptErrorCode::MissingColonAfterArgument,
            ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInFunctionCall,
            ValuascriptErrorCode::TrailingCommaInFunctionCall);

        try {
            const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterArguments);
            auto func_call = std::make_unique<FunctionCall>(std::move(target), std::move(arguments));
            func_call->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));
            return func_call;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightParen);
            auto func_call = std::make_unique<FunctionCall>(std::move(target), std::move(arguments));
            func_call->span = cursor_.combine_spans(target_span,
                                                    cursor_.make_span(cursor_.previous(), cursor_.previous()));
            return func_call;
        }
    }

    std::unique_ptr<Expression> Parser::parse_tensor_access(std::unique_ptr<Expression> target) {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(*this, TokenType::RightBracket);
        std::unique_ptr<Expression> index_expr = nullptr;

        try {
            auto parse_bound = [&]() -> std::unique_ptr<Expression> {
                if (cursor_.check(TokenType::Colon) || cursor_.check(TokenType::RightBracket)) {
                    return nullptr;
                }

                auto expr = parse_expression();

                if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::RightBracket)) {
                    if (TokenTraits::is_expression_start(cursor_.peek().type)) {
                        cursor_.report_error(cursor_.peek(),
                                             ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor);
                    } else if (cursor_.check(TokenType::Comma)) {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::UnexpectedCommaInBracketAccess);
                    }
                }

                return expr;
            };

            index_expr = parse_bound();

            if (cursor_.match({TokenType::Colon})) {
                std::unique_ptr<Expression> end_expr = parse_bound();
                const SourceSpan colon_span = index_expr ? index_expr->span : target_span;
                const SourceSpan slice_end_span = end_expr
                                                      ? end_expr->span
                                                      : cursor_.make_span(cursor_.previous(), cursor_.previous());

                index_expr = std::make_unique<BinaryExpression>(std::move(index_expr), TokenType::Colon,
                                                                std::move(end_expr));

                index_expr->span = cursor_.combine_spans(colon_span, slice_end_span);
            } else if (!index_expr) {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::EmptyBracketAccess);
            }

            const Token &end_token = cursor_.consume(TokenType::RightBracket,
                                                     ValuascriptErrorCode::UnmatchedBracketAfterTensorIndex);
            auto bracket_access = std::make_unique<BracketAccess>(std::move(target), std::move(index_expr));
            bracket_access->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));
            return bracket_access;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBracket);
            auto bracket_access = std::make_unique<BracketAccess>(std::move(target), std::move(index_expr));
            bracket_access->span = cursor_.combine_spans(target_span,
                                                         cursor_.make_span(cursor_.previous(), cursor_.previous()));
            return bracket_access;
        }
    }

    std::unique_ptr<Expression> Parser::parse_dot_access(std::unique_ptr<Expression> target) {
        const SourceSpan target_span = target->span;
        Token property_token = consume_identifier(ValuascriptErrorCode::ExpectedPropertyName, true, true);
        auto dot_access = std::make_unique<DotAccess>(std::move(target), property_token.lexeme);
        dot_access->span = cursor_.combine_spans(target_span, cursor_.make_span(property_token, property_token));
        return dot_access;
    }

    std::unique_ptr<Expression> Parser::parse_primary_expression() {
        const Token &tok = cursor_.peek();
        const Token &next = cursor_.peek(1);
        const Token &prev = cursor_.previous();

        switch (tok.type) {
            case TokenType::Number: {
                cursor_.advance();
                auto node = std::make_unique<NumberLiteral>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::PercentageLiteral: {
                cursor_.advance();
                auto node = std::make_unique<PercentageLiteral>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::String: {
                cursor_.advance();
                auto node = std::make_unique<StringLiteral>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::True:
            case TokenType::False: {
                cursor_.advance();
                auto node = std::make_unique<BooleanLiteral>(cursor_.previous().type == TokenType::True);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::Identifier: {
                cursor_.advance();
                auto node = std::make_unique<IdentifierAccess>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::Self: {
                cursor_.advance();
                auto node = std::make_unique<SelfExpression>();
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::Switch:
                cursor_.advance();
                return parse_switch_expression();
            case TokenType::If: {
                cursor_.advance();
                return parse_conditional_expression();
            }
            case TokenType::LeftParen:
                cursor_.advance();
                return parse_tuple_or_grouping();
            case TokenType::LeftBracket:
                cursor_.advance();
                return parse_tensor_literal();
            case TokenType::LeftBrace:
                cursor_.advance();
                return parse_dict_literal();
            default: {
                bool is_stmt_start = TokenTraits::is_statement_start(tok, next.type);
                bool force_location = (tok.type != TokenType::EndOfFile && !is_stmt_start);

                if (is_stmt_start) {
                    if (tok.line > prev.line && TokenTraits::is_dangling_operator(prev.type)) {
                        cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, force_location);
                    }

                    cursor_.report_error_no_panic(tok, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere, true);

                    if (tok.type == TokenType::At) {
                        cursor_.report_error_no_panic(tok, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration,
                                                      true);
                        parse_modifiers();
                    } else {
                        consume_unexpected_statement_gracefully();
                    }

                    if (TokenTraits::is_expression_start(cursor_.peek().type)) {
                        return parse_primary_expression();
                    }

                    throw ParseSyncException();
                }

                if (tok.type == TokenType::Case || tok.type == TokenType::Default ||
                    tok.type == TokenType::RightBrace || tok.type == TokenType::RightParen ||
                    tok.type == TokenType::RightBracket || tok.type == TokenType::Return ||
                    tok.type == TokenType::Then || tok.type == TokenType::Else) {
                    if (tok.line > prev.line && TokenTraits::is_dangling_operator(prev.type)) {
                        cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, false);
                    }

                    cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, force_location);
                }

                if (is_reserved_keyword(tok)) {
                    cursor_.report_error_no_panic(tok, ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
                    cursor_.advance();
                    auto node = std::make_unique<IdentifierAccess>(tok.lexeme);
                    node->span = cursor_.make_span(tok, tok);
                    return node;
                }

                cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, force_location);
            }
        }
    }

    std::unique_ptr<Expression> Parser::parse_tuple_or_grouping() {
        const Token &start_token = cursor_.previous();
        CloserTracker tracker(*this, TokenType::RightParen);

        if (cursor_.match({TokenType::RightParen})) {
            auto node = std::make_unique<TupleLiteral>(std::vector<std::unique_ptr<Expression> >{});
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }

        std::unique_ptr<Expression> first_expr = nullptr;
        bool first_expr_failed = false;

        try {
            first_expr = parse_expression();
        } catch (const ParseSyncException &) {
            first_expr_failed = true;

            int internal_depth = 0;
            while (!cursor_.is_at_end()) {
                const Token &tok = cursor_.peek();
                if (internal_depth == 0 && (tok.type == TokenType::Comma || tok.type == TokenType::RightParen)) break;
                if (TokenTraits::is_grouping_opener(tok.type)) internal_depth++;
                else if (TokenTraits::is_grouping_closer(tok.type)) internal_depth--;

                cursor_.advance();
                if (internal_depth < 0) break;
            }
        }

        if (cursor_.match({TokenType::Comma})) {
            return complete_tuple(std::move(first_expr), start_token);
        }

        return complete_grouping(std::move(first_expr), first_expr_failed, start_token);
    }

    std::unique_ptr<Expression>
    Parser::complete_tuple(std::unique_ptr<Expression> first_expr, const Token &start_token) {
        if (cursor_.check(TokenType::RightParen)) {
            cursor_.report_error_no_panic(cursor_.previous(), ValuascriptErrorCode::SingleElementTuplesNotAllowed);
        }

        std::vector<std::unique_ptr<Expression> > elements;
        if (first_expr) elements.push_back(std::move(first_expr));

        auto remaining = parse_expression_list(TokenType::RightParen, ValuascriptErrorCode::TrailingCommaInTuple);
        elements.insert(elements.end(), std::make_move_iterator(remaining.begin()),
                        std::make_move_iterator(remaining.end()));

        try {
            const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterTupleElements);
            auto node = std::make_unique<TupleLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightParen);
            auto node = std::make_unique<TupleLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }
    }

    std::unique_ptr<Expression> Parser::complete_grouping(std::unique_ptr<Expression> first_expr,
                                                          bool first_expr_failed, const Token &start_token) {
        try {
            if (!first_expr_failed && !cursor_.check(TokenType::RightParen) && TokenTraits::is_expression_start(
                    cursor_.peek().type)) {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperatorInsideGrouping);
            }

            const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterExpression);
            auto node = std::make_unique<GroupingExpression>(std::move(first_expr));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightParen);
            auto node = std::make_unique<GroupingExpression>(std::move(first_expr));
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }
    }

    std::unique_ptr<Expression> Parser::parse_tensor_literal() {
        const Token &start_token = cursor_.previous();
        CloserTracker tracker(*this, TokenType::RightBracket);
        auto elements = parse_expression_list(TokenType::RightBracket);

        try {
            const Token &end_token = cursor_.consume(TokenType::RightBracket,
                                                     ValuascriptErrorCode::UnmatchedBracketAfterTensorElements);
            auto node = std::make_unique<TensorLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBracket);
            auto node = std::make_unique<TensorLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }
    }

    std::unique_ptr<Expression> Parser::parse_dict_literal() {
        const Token &start_token = cursor_.previous();
        CloserTracker tracker(*this, TokenType::RightBrace);

        auto elements = parse_list<DictItem>(
            TokenType::RightBrace,
            std::nullopt,
            ValuascriptErrorCode::ExpectedCommaSeparatorInDictionaryLiteral,
            std::vector<TokenType>{},
            [this]() {
                const Token &tok = cursor_.peek();
                const TokenType next = cursor_.peek(1).type;

                if (tok.type == TokenType::At) {
                    if (is_at_any_declaration()) return false;
                }

                if (tok.type == TokenType::At || tok.type == TokenType::Identifier) return true;

                return is_reserved_keyword(tok) && (next == TokenType::Colon);
            },
            [this]() {
                auto modifiers = parse_modifiers();
                Token key_token = consume_identifier(ValuascriptErrorCode::ExpectedDictionaryKey, false);
                cursor_.consume(TokenType::Colon, ValuascriptErrorCode::ExpectedColonAfterDictionaryKey);

                std::unique_ptr<Expression> val = nullptr;
                if (cursor_.check(TokenType::Comma) || cursor_.check(TokenType::RightBrace)) {
                    cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::InvalidExpression);
                } else {
                    val = parse_expression();

                    if (TokenTraits::is_expression_start(cursor_.peek().type) && cursor_.peek(1).type !=
                        TokenType::Colon) {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                    }
                }

                return DictItem{std::move(modifiers), key_token.lexeme, std::move(val)};
            }
        );

        try {
            const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                     ValuascriptErrorCode::UnmatchedBraceInDictionaryLiteral);
            auto node = std::make_unique<DictLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBrace);
            auto node = std::make_unique<DictLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }
    }

    std::unique_ptr<Expression> Parser::parse_conditional_expression() {
        const Token &start_token = cursor_.previous();

        std::unique_ptr<Expression> condition = nullptr;
        try {
            condition = parse_expression();
        } catch (const ParseSyncException &) {
            synchronize_to_conditional_boundary();
        }

        std::unique_ptr<Expression> then_branch = nullptr;
        if (!cursor_.match({TokenType::Then})) {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingThenToken);
        }

        try {
            then_branch = parse_expression();
        } catch (const ParseSyncException &) {
            synchronize_to_conditional_boundary();
        }

        std::unique_ptr<Expression> else_branch = nullptr;
        if (!cursor_.match({TokenType::Else})) {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingElseToken);
        }

        else_branch = parse_expression();

        auto cond_expr = std::make_unique<ConditionalExpression>(std::move(condition), std::move(then_branch),
                                                                 std::move(else_branch));
        cond_expr->span = cursor_.make_span(start_token, cursor_.previous());
        return cond_expr;
    }

    std::unique_ptr<Expression> Parser::parse_switch_expression() {
        const Token &start_token = cursor_.previous();
        auto target = parse_switch_target();

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeSwitchBody);
        CloserTracker tracker(*this, TokenType::RightBrace);

        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases;
        std::unique_ptr<Expression> default_case = nullptr;

        parse_switch_body(cases, default_case);

        try {
            const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                     ValuascriptErrorCode::ExpectedRightBraceAfterSwitchBody);
            auto node = std::make_unique<
                SwitchExpression>(std::move(target), std::move(cases), std::move(default_case));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBrace);
            auto node = std::make_unique<
                SwitchExpression>(std::move(target), std::move(cases), std::move(default_case));
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }
    }

    std::pair<std::vector<std::string>, std::unique_ptr<Expression> > Parser::parse_switch_case() {
        std::vector<std::string> identifiers;

        while (true) {
            const Token &tok = cursor_.peek();
            if (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, cursor_.peek(1).type)) {
                Token id_token = consume_identifier(ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase);
                identifiers.push_back(id_token.lexeme);
            } else {
                cursor_.report_error_no_panic(tok, ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase, true);
                if (!cursor_.check(TokenType::Comma) && !cursor_.check(TokenType::Arrow)) {
                    throw ParseSyncException();
                }
            }

            if (cursor_.match({TokenType::Comma})) continue;
            if (cursor_.check(TokenType::Identifier)) {
                cursor_.report_error_no_panic(cursor_.peek(),
                                              ValuascriptErrorCode::ExpectedCommaBetweenCaseIdentifiers);
                continue;
            }
            break;
        }

        std::unique_ptr<Expression> result = nullptr;
        try {
            result = parse_switch_result();
        } catch (const ParseSyncException &) {
            synchronize_to_switch_boundary();
        }

        return {std::move(identifiers), std::move(result)};
    }

    std::unique_ptr<Expression> Parser::parse_switch_default() {
        std::unique_ptr<Expression> result = nullptr;
        try {
            result = parse_switch_result();
        } catch (const ParseSyncException &) {
            synchronize_to_switch_boundary();
        }
        return result;
    }

    std::unique_ptr<Expression> Parser::parse_switch_result() {
        cursor_.consume(TokenType::Arrow, ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier);
        auto expr = parse_expression();

        bool should_break_out = is_missing_closing_brace() &&
                                (is_at_top_level_declaration() || cursor_.peek().type == TokenType::Return ||
                                 TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                                 (cursor_.peek().line > cursor_.previous().line &&
                                  TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type)));

        if (!should_break_out && !cursor_.check(TokenType::Case) && !cursor_.check(TokenType::Default) &&
            !cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            if (cursor_.peek().line == cursor_.previous().line) {
                if (TokenTraits::is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::MissingOperatorInSwitchCaseResult, true);
                } else {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::CaseOrDefaultMissingInSwitchAfterResult, true);
                }
            }
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_switch_target() {
        try {
            cursor_.consume(TokenType::LeftParen, ValuascriptErrorCode::ExpectedLeftParenAfterSwitch);
            CloserTracker tracker(*this, TokenType::RightParen);
            auto target = parse_expression();
            cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::ExpectedRightParenAfterSwitchTarget);
            return target;
        } catch (const ParseSyncException &) {
            int depth = 0;
            while (!cursor_.is_at_end()) {
                TokenType type = cursor_.peek().type;
                if (depth == 0 && (type == TokenType::RightParen || type == TokenType::LeftBrace)) break;
                if (TokenTraits::is_grouping_opener(type)) depth++;
                else if (TokenTraits::is_grouping_closer(type)) depth--;
                cursor_.advance();
            }

            if (cursor_.check(TokenType::RightParen)) {
                cursor_.advance();
            } else if (TokenTraits::is_grouping_closer(cursor_.peek().type)) {
                if (!is_active_closer(cursor_.peek().type)) cursor_.advance();
            }
            return nullptr;
        }
    }

    void Parser::parse_switch_body(
        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > &cases,
        std::unique_ptr<Expression> &default_case) {
        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            bool should_break_out = is_missing_closing_brace() &&
                                    (is_at_top_level_declaration() || cursor_.peek().type == TokenType::Return ||
                                     TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                                     (cursor_.peek().line > cursor_.previous().line &&
                                      TokenTraits::is_expression_statement_start(
                                          cursor_.peek(), cursor_.peek(1).type)));

            if (should_break_out) break;

            try {
                if (cursor_.match({TokenType::Case})) {
                    cases.push_back(parse_switch_case());
                } else if (cursor_.match({TokenType::Default})) {
                    if (default_case != nullptr) {
                        cursor_.report_error_no_panic(cursor_.previous(),
                                                      ValuascriptErrorCode::MultipleDefaultCasesInSwitch);
                    }
                    default_case = parse_switch_default();
                } else {
                    if (TokenTraits::is_top_level_token(cursor_.peek().type)) {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere,
                                             true);
                    } else {
                        cursor_.report_error(cursor_.peek(),
                                             ValuascriptErrorCode::ExpectedCaseOrDefaultInsideSwitchBody, true);
                    }
                }
            } catch (const ParseSyncException &) {
                synchronize_to_switch_boundary();
            }
        }
    }
}
