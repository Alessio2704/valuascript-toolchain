#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    std::unique_ptr<Expression> Parser::parse_expression(const Precedence precedence) {
        auto left = parse_unary_expression();

        while (get_operator_precedence(cursor_.peek().type) >= precedence) {
            Token op = cursor_.advance();
            Precedence op_precedence = get_operator_precedence(op.type);

            const Precedence next_precedence = is_right_associative(op.type)
                                                   ? op_precedence
                                                   : static_cast<Precedence>(static_cast<int>(op_precedence) + 1);

            auto right = parse_expression(next_precedence);
            const SourceSpan combined = cursor_.combine_spans(left->span, right->span);

            left = std::make_unique<BinaryExpression>(std::move(left), op.type, std::move(right));
            left->span = combined;

            if (op_precedence == Precedence::Comparison &&
                get_operator_precedence(cursor_.peek().type) == Precedence::Comparison) {
                cursor_.report_error(cursor_.peek(), ErrorCode::ChainingNotAllowedForComparisonOperations,
                                     "Syntax Error: Chaining comparison operators is not allowed.");
            }
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parse_unary_expression() {
        if (cursor_.match({TokenType::Minus, TokenType::Plus, TokenType::Not})) {
            Token op = cursor_.previous();
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
        SourceSpan target_span = target->span;
        if (!cursor_.check(TokenType::RightParen) && !cursor_.is_at_end()) {
            const TokenType p0 = cursor_.peek().type;
            const TokenType p1 = cursor_.peek(1).type;

            if (p0 == TokenType::Identifier) {
                if (p1 != TokenType::Colon && is_binary_operator(p1)) {
                    cursor_.report_error(cursor_.previous(), ErrorCode::MissingOperatorOrArgumentName,
                                         "Syntax Error: Missing operator (like '*') before '(', or missing argument name.");
                }
            } else {
                if (is_expression_start(p0)) {
                    cursor_.report_error(cursor_.previous(), ErrorCode::MissingOperatorOrArgumentName,
                                         "Syntax Error: Missing operator (like '*') before '(', or expected argument name.");
                } else {
                    cursor_.report_error(cursor_.peek(), ErrorCode::MissingArgumentName,
                                         "Syntax Error: Expected an argument name (e.g., 'name: value') or a closing ')'.");
                }
            }
        }

        auto arguments = parse_key_value_list(
            TokenType::RightParen,
            ErrorCode::MissingArgumentName, "Expected argument name in function call.",
            ErrorCode::MissingColonAfterArgument, "Expected ':' after argument name.",
            ErrorCode::MissingCommaSeparatorForArgumentsInFunctionCall,
            ErrorCode::TrailingCommaInFunctionCall);

        const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::ExpectedRightParen,
                                                 "Expected ')' after arguments.");

        auto func_call = std::make_unique<FunctionCall>(std::move(target), std::move(arguments));
        func_call->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));
        return func_call;
    }

    std::unique_ptr<Expression> Parser::parse_tensor_access(std::unique_ptr<Expression> target) {
        SourceSpan target_span = target->span;

        auto parse_bound = [&]() -> std::unique_ptr<Expression> {
            if (cursor_.check(TokenType::Colon) || cursor_.check(TokenType::RightBracket)) {
                return nullptr;
            }

            auto expr = parse_expression();

            if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::RightBracket)) {
                if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator,
                                         "Syntax Error: Missing operator or expected ':' or ']' in tensor access.");
                } else if (cursor_.check(TokenType::Comma)) {
                    cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator,
                                         "Syntax Error: Unexpected ',' inside bracket access. If you meant to write a second tensor, you are missing an operator (like '+') between them.");
                }
            }
            return expr;
        };

        std::unique_ptr<Expression> index_expr = parse_bound();

        if (cursor_.match({TokenType::Colon})) {
            std::unique_ptr<Expression> end_expr = parse_bound();

            SourceSpan colon_span = index_expr ? index_expr->span : target_span;
            SourceSpan slice_end_span = end_expr
                                            ? end_expr->span
                                            : cursor_.make_span(cursor_.previous(), cursor_.previous());

            index_expr = std::make_unique<BinaryExpression>(std::move(index_expr), TokenType::Colon,
                                                            std::move(end_expr));
            index_expr->span = cursor_.combine_spans(colon_span, slice_end_span);
        } else if (!index_expr) {
            cursor_.report_error(cursor_.previous(), ErrorCode::EmptyBracketAccess,
                                 "Expected an index or slice inside '[]'.");
        }

        const Token &end_token = cursor_.consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                                 "Expected ']' after tensor index.");

        auto bracket_access = std::make_unique<BracketAccess>(std::move(target), std::move(index_expr));
        bracket_access->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));
        return bracket_access;
    }

    std::unique_ptr<Expression> Parser::parse_dot_access(std::unique_ptr<Expression> target) {
        const SourceSpan target_span = target->span;
        Token property_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedPropertyName,
                                               "Expected property name after '.'.");
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
                cursor_.report_error(cursor_.peek(), ErrorCode::InvalidExpression,
                                     "Syntax Error: Expected an expression.");
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
                cursor_.report_error(cursor_.previous(), ErrorCode::SingleElementTuplesNotAllowed,
                                     "Syntax Error: Trailing commas and 1-element tuples are not allowed.");
            }

            std::vector<std::unique_ptr<Expression> > elements;
            elements.push_back(std::move(first_expr));

            auto remaining_elements = parse_expression_list(TokenType::RightParen, ErrorCode::TrailingCommaInTuple);

            elements.insert(elements.end(), std::make_move_iterator(remaining_elements.begin()),
                            std::make_move_iterator(remaining_elements.end()));

            const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesisInTuple,
                                                     "Expected ')' after tuple elements.");
            auto node = std::make_unique<TupleLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        }

        if (!cursor_.check(TokenType::RightParen) && is_expression_start(cursor_.peek().type)) {
            cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator,
                                 "Syntax Error: Missing operator between expressions inside grouping.");
        }

        const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::ExpectedRightParen,
                                                 "Expected ')' after expression.");
        first_expr->span = cursor_.make_span(start_token, end_token);
        return first_expr;
    }

    std::unique_ptr<Expression> Parser::parse_tensor_literal() {
        const Token &start_token = cursor_.previous();

        auto elements = parse_expression_list(TokenType::RightBracket);

        const Token &end_token = cursor_.consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                                 "Expected ']' after vector elements.");

        auto node = std::make_unique<TensorLiteral>(std::move(elements));
        node->span = cursor_.make_span(start_token, end_token);
        return node;
    }

    std::unique_ptr<Expression> Parser::parse_dict_literal() {
        const Token &start_token = cursor_.previous();

        auto pairs = parse_key_value_list(
            TokenType::RightBrace,
            ErrorCode::ExpectedDictionaryKey, "Expected key in dictionary.",
            ErrorCode::ExpectedColonAfterDictionaryKey, "Expected ':' after dictionary key.",
            ErrorCode::ExpectedCommaSeparatorInDictionaryLiteral);

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::UnmatchedBraceInDictionaryLiteral,
                                                 "Expected '}' after dictionary literal.");
        auto node = std::make_unique<DictLiteral>(std::move(pairs));
        node->span = cursor_.make_span(start_token, end_token);
        return node;
    }

    std::unique_ptr<Expression> Parser::parse_conditional_expression() {
        const Token &start_token = cursor_.previous();
        auto condition = parse_expression();
        cursor_.consume(TokenType::Then, ErrorCode::MissingThenToken, "Expected 'then'.");
        auto then_branch = parse_expression();
        cursor_.consume(TokenType::Else, ErrorCode::MissingElseToken, "Expected 'else'.");
        auto else_branch = parse_expression();

        auto cond_expr = std::make_unique<ConditionalExpression>(
            std::move(condition), std::move(then_branch), std::move(else_branch));
        cond_expr->span = cursor_.make_span(start_token, cursor_.previous());
        return cond_expr;
    }

    std::unique_ptr<Expression> Parser::parse_switch_expression() {
        const Token &start_token = cursor_.previous();
        cursor_.consume(TokenType::LeftParen, ErrorCode::ExpectedLeftParen, "Expected '(' after 'switch'.");
        auto target = parse_expression();
        cursor_.consume(TokenType::RightParen, ErrorCode::ExpectedRightParen, "Expected ')' after switch target.");
        cursor_.consume(TokenType::LeftBrace, ErrorCode::ExpectedLeftBrace, "Expected '{' before switch body.");

        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases;
        std::unique_ptr<Expression> default_case = nullptr;

        auto parse_branch_result = [&]() -> std::unique_ptr<Expression> {
            cursor_.consume(TokenType::Arrow, ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier,
                            "Expected '->' before case result.");
            auto expr = parse_expression();

            if (!cursor_.check(TokenType::Case) && !cursor_.check(TokenType::Default) && !cursor_.
                check(TokenType::RightBrace) && !cursor_.is_at_end()) {
                if (is_expression_start(cursor_.peek().type)) {
                    cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator,
                                         "Syntax Error: Missing operator between expressions in switch case result.");
                } else {
                    cursor_.report_error(cursor_.peek(), ErrorCode::CaseOrDefaultMissingInSwitch,
                                         "Syntax Error: Expected 'case', 'default', or '}' after case result.");
                }
            }
            return expr;
        };

        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            if (cursor_.match({TokenType::Case})) {
                std::vector<std::string> case_identifiers;
                do {
                    Token id_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedEnumCaseName,
                                                     "Expected enum case identifier after 'case'.");
                    case_identifiers.push_back(id_token.lexeme);

                    if (cursor_.match({TokenType::Comma})) {
                    } else if (cursor_.check(TokenType::Identifier)) {
                        cursor_.report_error(cursor_.peek(), ErrorCode::MissingOperator,
                                             "Syntax Error: Missing comma ',' between case identifiers.");
                    } else {
                        break;
                    }
                } while (true);

                auto result_expr = parse_branch_result();
                cases.emplace_back(std::move(case_identifiers), std::move(result_expr));
            } else if (cursor_.match({TokenType::Default})) {
                if (default_case != nullptr) {
                    cursor_.report_error(cursor_.peek(), ErrorCode::MultipleDefaultCasesInSwitch,
                                         "Syntax Error: A switch expression can only have one 'default' case.");
                }

                default_case = parse_branch_result();
            } else {
                cursor_.report_error(cursor_.peek(), ErrorCode::CaseOrDefaultMissingInSwitch,
                                     "Syntax Error: Expected 'case' or 'default' inside switch body.");
            }
        }

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::ExpectedRightBrace,
                                                 "Expected '}' after switch body.");
        auto switch_expr = std::make_unique<SwitchExpression>(std::move(target), std::move(cases),
                                                              std::move(default_case));
        switch_expr->span = cursor_.make_span(start_token, end_token);
        return switch_expr;
    }
}
