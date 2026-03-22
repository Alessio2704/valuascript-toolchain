#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    std::unique_ptr<Expression> Parser::parse_expression(const Precedence precedence) {
        auto left = parse_unary_expression();

        while (get_operator_precedence(cursor_.peek().type) >= precedence) {
            Token op = cursor_.advance();

            Precedence op_precedence = get_operator_precedence(op.type);

            const Precedence next_precedence = is_operator_right_associative(op.type)
                                                   ? op_precedence
                                                   : static_cast<Precedence>(static_cast<int>(op_precedence) + 1);

            auto right = parse_expression(next_precedence);

            const SourceSpan combined = cursor_.combine_spans(left->span, right->span);

            left = std::make_unique<BinaryExpression>(std::move(left), op.type, std::move(right));

            left->span = combined;

            if (op_precedence == Precedence::Comparison &&
                get_operator_precedence(cursor_.peek().type) == Precedence::Comparison) {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations);
            }
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parse_unary_expression() {
        if (is_unary_operator(cursor_.peek().type)) {
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

        if (!cursor_.check(TokenType::RightParen) && !cursor_.is_at_end()) {
            const TokenType p0 = cursor_.peek().type;
            const TokenType p1 = cursor_.peek(1).type;

            if (p0 == TokenType::Identifier) {
                if (p1 != TokenType::Colon && is_binary_operator(p1)) {
                    cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MissingOperatorOrArgumentName);
                }
            } else {
                if (is_expression_start(p0)) {
                    cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MissingOperatorOrArgumentName);
                } else {
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

        const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                 ValuascriptErrorCode::ExpectedRightParenAfterArguments);

        auto func_call = std::make_unique<FunctionCall>(std::move(target), std::move(arguments));

        func_call->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));

        return func_call;
    }

    std::unique_ptr<Expression> Parser::parse_tensor_access(std::unique_ptr<Expression> target) {
        const SourceSpan target_span = target->span;

        auto parse_bound = [&]() -> std::unique_ptr<Expression> {
            if (cursor_.check(TokenType::Colon) || cursor_.check(TokenType::RightBracket)) {
                return nullptr;
            }

            auto expr = parse_expression();

            if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::RightBracket)) {
                if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(),
                                         ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor);
                } else if (cursor_.check(TokenType::Comma)) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::UnexpectedCommaInBracketAccess);
                }
            }

            return expr;
        };

        std::unique_ptr<Expression> index_expr = parse_bound();

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
    }

    std::unique_ptr<Expression> Parser::parse_dot_access(std::unique_ptr<Expression> target) {
        const SourceSpan target_span = target->span;

        Token property_token = consume_identifier(ValuascriptErrorCode::ExpectedPropertyName);

        auto dot_access = std::make_unique<DotAccess>(std::move(target), property_token.lexeme);

        dot_access->span = cursor_.combine_spans(target_span, cursor_.make_span(property_token, property_token));

        return dot_access;
    }

    std::unique_ptr<Expression> Parser::parse_primary_expression() {
        switch (cursor_.peek().type) {
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
            default:
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::InvalidExpression);
        }
    }

    std::unique_ptr<Expression> Parser::parse_tuple_or_grouping() {
        const Token &start_token = cursor_.previous();

        if (cursor_.match({TokenType::RightParen})) {
            auto node = std::make_unique<TupleLiteral>(std::vector<std::unique_ptr<Expression> >{});
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }

        auto first_expr = parse_expression();

        if (cursor_.match({TokenType::Comma})) {
            if (cursor_.check(TokenType::RightParen)) {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::SingleElementTuplesNotAllowed);
            }

            std::vector<std::unique_ptr<Expression> > elements;
            elements.push_back(std::move(first_expr));
            auto remaining_elements = parse_expression_list(TokenType::RightParen,
                                                            ValuascriptErrorCode::TrailingCommaInTuple);

            elements.insert(elements.end(), std::make_move_iterator(remaining_elements.begin()),
                            std::make_move_iterator(remaining_elements.end()));

            const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterTupleElements);

            auto node = std::make_unique<TupleLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        }

        if (!cursor_.check(TokenType::RightParen) && is_expression_start(cursor_.peek().type)) {
            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperatorInsideGrouping);
        }

        const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                 ValuascriptErrorCode::ExpectedRightParenAfterExpression);

        first_expr->span = cursor_.make_span(start_token, end_token);
        return first_expr;
    }

    std::unique_ptr<Expression> Parser::parse_tensor_literal() {
        const Token &start_token = cursor_.previous();

        auto elements = parse_expression_list(TokenType::RightBracket);

        const Token &end_token = cursor_.consume(TokenType::RightBracket,
                                                 ValuascriptErrorCode::UnmatchedBracketAfterVectorElements);

        auto node = std::make_unique<TensorLiteral>(std::move(elements));

        node->span = cursor_.make_span(start_token, end_token);

        return node;
    }

    std::unique_ptr<Expression> Parser::parse_dict_literal() {
        const Token &start_token = cursor_.previous();

        auto pairs = parse_key_value_list(
            TokenType::RightBrace,
            ValuascriptErrorCode::ExpectedDictionaryKey,
            ValuascriptErrorCode::ExpectedColonAfterDictionaryKey,
            ValuascriptErrorCode::ExpectedCommaSeparatorInDictionaryLiteral);

        const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                 ValuascriptErrorCode::UnmatchedBraceInDictionaryLiteral);

        auto node = std::make_unique<DictLiteral>(std::move(pairs));
        node->span = cursor_.make_span(start_token, end_token);

        return node;
    }

    std::unique_ptr<Expression> Parser::parse_conditional_expression() {
        const Token &start_token = cursor_.previous();

        auto condition = parse_expression();

        cursor_.consume(TokenType::Then, ValuascriptErrorCode::MissingThenToken);

        auto then_branch = parse_expression();

        cursor_.consume(TokenType::Else, ValuascriptErrorCode::MissingElseToken);

        auto else_branch = parse_expression();

        auto cond_expr = std::make_unique<ConditionalExpression>(
            std::move(condition), std::move(then_branch), std::move(else_branch));

        cond_expr->span = cursor_.make_span(start_token, cursor_.previous());

        return cond_expr;
    }

    std::unique_ptr<Expression> Parser::parse_switch_expression() {
        const Token &start_token = cursor_.previous();

        cursor_.consume(TokenType::LeftParen, ValuascriptErrorCode::ExpectedLeftParenAfterSwitch);

        auto target = parse_expression();

        cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::ExpectedRightParenAfterSwitchTarget);

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeSwitchBody);

        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases;
        std::unique_ptr<Expression> default_case = nullptr;

        auto parse_branch_result = [&]() -> std::unique_ptr<Expression> {
            cursor_.consume(TokenType::Arrow, ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier);

            auto expr = parse_expression();

            if (!cursor_.check(TokenType::Case) && !cursor_.check(TokenType::Default) && !cursor_.
                check(TokenType::RightBrace) && !cursor_.is_at_end()) {
                if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperatorInSwitchCaseResult);
                } else {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::CaseOrDefaultMissingInSwitchAfterResult);
                }
            }
            return expr;
        };

        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            if (cursor_.match({TokenType::Case})) {
                std::vector<std::string> case_identifiers;

                do {
                    Token id_token = consume_identifier(ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase);

                    case_identifiers.push_back(id_token.lexeme);

                    if (cursor_.match({TokenType::Comma})) {
                    } else if (cursor_.check(TokenType::Identifier)) {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ExpectedCommaBetweenCaseIdentifiers);
                    } else {
                        break;
                    }
                } while (true);

                auto result_expr = parse_branch_result();

                cases.emplace_back(std::move(case_identifiers), std::move(result_expr));
            } else if (cursor_.match({TokenType::Default})) {
                if (default_case != nullptr) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MultipleDefaultCasesInSwitch);
                }

                default_case = parse_branch_result();
            } else {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ExpectedCaseOrDefaultInsideSwitchBody);
            }
        }

        const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                 ValuascriptErrorCode::ExpectedRightBraceAfterSwitchBody);

        auto switch_expr = std::make_unique<SwitchExpression>(std::move(target), std::move(cases),
                                                              std::move(default_case));

        switch_expr->span = cursor_.make_span(start_token, end_token);

        return switch_expr;
    }
}
