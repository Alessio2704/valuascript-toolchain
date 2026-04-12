#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    std::unique_ptr<ImportStatement> Parser::parse_import_statement() {
        const Token &start_token = cursor_.consume(TokenType::Import, ValuascriptErrorCode::ExpectedImportToken);
        const Token &path = cursor_.consume(TokenType::String, ValuascriptErrorCode::MissingImportPathString);
        auto stmt = std::make_unique<ImportStatement>(path.lexeme);
        stmt->span = cursor_.make_span(start_token, path);
        return stmt;
    }

    std::unique_ptr<Directive> Parser::parse_directive() {
        const Token &start_token = cursor_.consume(TokenType::Hash, ValuascriptErrorCode::ExpectedHashToken);
        const Token &name_token = consume_identifier(ValuascriptErrorCode::MissingDirectiveName);
        std::string directive_name = name_token.lexeme;

        std::unique_ptr<Expression> value = nullptr;

        if (cursor_.match({TokenType::Assign})) {
            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                                  (cursor_.peek().line > cursor_.previous().line &&
                                   cursor_.peek().type == TokenType::Identifier &&
                                   cursor_.peek(1).type == TokenType::Assign);

            if (cursor_.is_at_end() || is_pseudo_stmt) {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals, false);
            } else {
                value = parse_expression();
            }
        } else if (cursor_.peek().line == cursor_.previous().line && TokenTraits::is_expression_start(
                       cursor_.peek().type)) {
            value = parse_expression();
        }

        if (value) {
            verify_statement_end();
        }

        auto dir = std::make_unique<Directive>(directive_name, std::move(value));
        dir->span = cursor_.make_span(start_token, cursor_.previous());
        return dir;
    }

    std::vector<Modifier> Parser::parse_modifiers(bool is_statement_context) {
        std::vector<Modifier> modifiers;

        while (cursor_.match({TokenType::At})) {
            const Token &start_token = cursor_.previous();

            try {
                Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedModifierName, is_statement_context);

                std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

                if (cursor_.match({TokenType::LeftParen})) {
                    CloserTracker tracker(*this, TokenType::RightParen);
                    arguments = parse_key_value_list(
                        TokenType::RightParen,
                        ValuascriptErrorCode::MissingArgumentNameInModifier,
                        ValuascriptErrorCode::MissingColonAfterArgument,
                        ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInModifier,
                        std::make_optional(ValuascriptErrorCode::TrailingCommaInModifier));

                    try {
                        cursor_.consume(TokenType::RightParen,
                                        ValuascriptErrorCode::UnmatchedParenthesisAfterModifierArgs);
                    } catch (const ParseSyncException &) {
                        synchronize_and_consume_closer(TokenType::RightParen);

                        Modifier mod;
                        mod.name = name_token.lexeme;
                        mod.arguments = std::move(arguments);
                        mod.span = cursor_.make_span(start_token, cursor_.previous());
                        modifiers.push_back(std::move(mod));

                        if (is_active_closer(cursor_.peek().type) && cursor_.peek().type != TokenType::RightParen) {
                            throw ParseSyncException();
                        }
                        continue;
                    }
                }

                Modifier mod;
                mod.name = name_token.lexeme;
                mod.arguments = std::move(arguments);
                mod.span = cursor_.make_span(start_token, cursor_.previous());
                modifiers.push_back(std::move(mod));
            } catch (const ParseSyncException &) {
                if (is_active_closer(cursor_.peek().type)) {
                    throw;
                }

                if (cursor_.is_at_end() ||
                    TokenTraits::is_grouping_closer(cursor_.peek().type) ||
                    TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type)) {
                    break;
                }

                if (cursor_.peek().type != TokenType::At) {
                    cursor_.advance();
                }
            }
        }

        return modifiers;
    }

    std::unique_ptr<StructDefinition> Parser::parse_struct_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Struct, ValuascriptErrorCode::ExpectedStructToken);
        Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedStructName);
        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedBraceInStructDefinition);
        CloserTracker tracker(*this, TokenType::RightBrace);

        auto is_at_parent_boundary = [this](int offset = 0) {
            const Token &tok = cursor_.peek(offset);
            const TokenType next = cursor_.peek(offset + 1).type;
            return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next ==
                   TokenType::Colon;
        };

        auto fields = parse_list<StructField>(
            TokenType::RightBrace,
            std::nullopt,
            std::make_optional(ValuascriptErrorCode::ExpectedCommaSeparatorInStruct),
            std::vector<TokenType>{},
            [this]() {
                const Token &tok = cursor_.peek();
                const TokenType next = cursor_.peek(1).type;

                if (tok.type == TokenType::At) {
                    if (is_at_any_declaration()) return false;
                    return true;
                }

                if (tok.type == TokenType::Identifier) return true;

                return is_reserved_keyword(tok) && (next == TokenType::Colon);
            },
            [&]() {
                const Token &field_start = cursor_.peek();
                auto field_mods = parse_modifiers();

                Token field_name = consume_identifier(ValuascriptErrorCode::ExpectedStructFieldName, false);
                cursor_.consume(TokenType::Colon, ValuascriptErrorCode::ExpectedColonAfterStructFieldName);
                auto type = parse_type_annotation(is_at_parent_boundary);

                SourceSpan span = cursor_.make_span(field_start, cursor_.previous());
                return StructField{std::move(field_mods), field_name.lexeme, std::move(type), span};
            }
        );

        Token end_token = cursor_.previous();
        try {
            end_token = cursor_.consume(TokenType::RightBrace, ValuascriptErrorCode::ExpectedRightBraceAfterStructBody);
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBrace);
            end_token = cursor_.previous();
        }

        auto struct_def = std::make_unique<
            StructDefinition>(std::move(modifiers), name_token.lexeme, std::move(fields));
        struct_def->span = cursor_.make_span(start_token, end_token);
        return struct_def;
    }

    std::unique_ptr<EnumDefinition> Parser::parse_enum_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Enum, ValuascriptErrorCode::ExpectedEnumToken);
        Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedEnumName);
        cursor_.consume(TokenType::Colon, ValuascriptErrorCode::ExpectedColonAfterEnumName);
        auto underlying_type = parse_type_annotation();
        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeEnumBody);
        CloserTracker tracker(*this, TokenType::RightBrace);

        auto cases = parse_list<EnumCase>(
            TokenType::RightBrace,
            std::nullopt,
            std::make_optional(ValuascriptErrorCode::ExpectedCommaSeparatorInEnum),
            std::vector<TokenType>{},
            [this]() {
                const Token &tok = cursor_.peek();
                const Token &next = cursor_.peek(1);

                if (tok.type == TokenType::At) {
                    if (is_at_any_declaration()) return false;
                    return true;
                }

                if (tok.type == TokenType::At || tok.type == TokenType::Identifier) return true;

                if (is_reserved_keyword(tok)) {
                    if (TokenTraits::is_top_level_only_declaration(tok.type)) return false;
                    if (tok.type == TokenType::Let || tok.type == TokenType::Var) {
                        return next.type != TokenType::Identifier;
                    }

                    return true;
                }
                return false;
            },
            [this]() {
                auto case_modifiers = parse_modifiers();
                Token case_name = consume_identifier(ValuascriptErrorCode::ExpectedEnumCaseName, false);

                std::unique_ptr<Expression> raw_value = nullptr;
                if (cursor_.match({TokenType::Assign})) {
                    raw_value = parse_expression();

                    if (TokenTraits::is_expression_start(cursor_.peek().type) && cursor_.peek(1).type !=
                        TokenType::Assign) {
                        if (!TokenTraits::is_newline_statement_boundary(cursor_.previous(), cursor_.peek(),
                                                                        cursor_.peek(1).type)) {
                            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                        }
                    }
                }
                return EnumCase{std::move(case_modifiers), case_name.lexeme, std::move(raw_value)};
            }
        );

        Token end_token = cursor_.previous();
        try {
            end_token = cursor_.consume(TokenType::RightBrace, ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody);
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBrace);
            end_token = cursor_.previous();
        }

        auto enum_def = std::make_unique<EnumDefinition>(std::move(modifiers), name_token.lexeme,
                                                         std::move(underlying_type), std::move(cases));
        enum_def->span = cursor_.make_span(start_token, end_token);
        return enum_def;
    }

    std::unique_ptr<FunctionDefinition> Parser::parse_function_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Func, ValuascriptErrorCode::ExpectedFuncToken);
        const Token &name = consume_identifier(ValuascriptErrorCode::MissingFunctionName);
        cursor_.consume(TokenType::LeftParen, ValuascriptErrorCode::ExpectedLeftParenAfterFunctionName);

        auto is_at_parent_boundary = [this](int offset = 0) {
            const Token &tok = cursor_.peek(offset);
            const TokenType next = cursor_.peek(offset + 1).type;
            return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next ==
                   TokenType::Colon;
        };

        std::vector<FunctionParameter> params;
        {
            CloserTracker param_tracker(*this, TokenType::RightParen);
            bool seen_default_param = false;

            params = parse_list<FunctionParameter>(
                TokenType::RightParen,
                std::make_optional(ValuascriptErrorCode::TrailingCommaInFunctionCall),
                std::make_optional(ValuascriptErrorCode::ExpectedCommaSeparatorInParameterList),
                std::vector<TokenType>{},
                [this]() {
                    const Token &tok = cursor_.peek();
                    return tok.type == TokenType::At || tok.type == TokenType::Identifier ||
                           TokenTraits::acts_like_identifier(tok, cursor_.peek(1).type);
                },
                [&]() {
                    auto mods = parse_modifiers();
                    const Token &param_name = consume_identifier(ValuascriptErrorCode::MissingParameterName);
                    cursor_.consume(TokenType::Colon, ValuascriptErrorCode::MissingColonAfterParameter);
                    auto type = parse_type_annotation(is_at_parent_boundary);

                    std::unique_ptr<Expression> default_value = nullptr;
                    if (cursor_.match({TokenType::Assign})) {
                        if (cursor_.check(TokenType::Comma) || cursor_.check(TokenType::RightParen)) {
                            cursor_.report_error(cursor_.previous(),
                                                 ValuascriptErrorCode::MissingDefaultParameterValue);
                        }
                        default_value = parse_expression();
                        seen_default_param = true;
                    } else if (seen_default_param) {
                        cursor_.report_error_no_panic(
                            param_name, ValuascriptErrorCode::NonDefaultParameterAfterDefault);
                    }

                    return FunctionParameter{
                        std::move(mods), param_name.lexeme, std::move(type), std::move(default_value)
                    };
                }
            );
        }

        cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::ExpectedRightParenAfterParameters);
        cursor_.consume(TokenType::Arrow, ValuascriptErrorCode::MissingArrowInFunction);

        auto return_types = parse_list<std::unique_ptr<TypeAnnotation> >(
            TokenType::LeftBrace,
            std::make_optional(ValuascriptErrorCode::TrailingComma),
            ValuascriptErrorCode::ExpectedCommaSeparatorInReturnTypeList,
            std::vector<TokenType>{},
            [&]() { return parse_type_annotation(); }
        );

        if (return_types.empty()) {
            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingTypeAnnotationAfterArrow);
        }

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeFunctionBody);

        CloserTracker body_tracker(*this, TokenType::RightBrace);

        std::optional<std::string> docstring = std::nullopt;
        if (cursor_.check(TokenType::DocString)) {
            docstring = cursor_.advance().lexeme;
        }

        std::vector<std::unique_ptr<Statement> > body;
        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            if (is_at_top_level_declaration() && is_missing_closing_brace()) {
                break;
            }
            try {
                parse_statement_or_declaration(ParseContext::FunctionBody, nullptr, body);
            } catch (const ParseSyncException &) {
                synchronize_block_statement();
            }
        }

        Token end_token = cursor_.previous();
        try {
            end_token = cursor_.consume(TokenType::RightBrace,
                                        ValuascriptErrorCode::ExpectedRightBraceAfterFunctionBody);
        } catch (const ParseSyncException &) {
            synchronize_and_consume_closer(TokenType::RightBrace);
            end_token = cursor_.previous();
        }

        auto func_def = std::make_unique<FunctionDefinition>(std::move(modifiers), name.lexeme, std::move(params),
                                                             std::move(return_types), std::move(body),
                                                             std::move(docstring));
        func_def->span = cursor_.make_span(start_token, end_token);
        return func_def;
    }

    std::unique_ptr<TypeAnnotation> Parser::parse_type_annotation(ParentBoundaryPredicate is_at_parent_boundary) {
        const Token &start_token = cursor_.peek();

        if (cursor_.match({TokenType::LeftParen})) {
            CloserTracker tracker(*this, TokenType::RightParen);

            auto elements = parse_list<std::unique_ptr<TypeAnnotation> >(
                TokenType::RightParen,
                std::make_optional(ValuascriptErrorCode::SingleElementTuplesNotAllowed),
                ValuascriptErrorCode::ExpectedCommaSeparatorInTupleType,
                std::vector<TokenType>{},
                [&]() { return parse_type_annotation(is_at_parent_boundary); },
                is_at_parent_boundary
            );

            Token end_token = cursor_.previous();
            try {
                end_token = cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::UnmatchedParenthesisInTuple);
            } catch (const ParseSyncException &) {
                TokenType peek_type = cursor_.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) &&
                    !is_active_closer(peek_type)) {
                    end_token = cursor_.advance();
                } else {
                    end_token = cursor_.previous();
                }
            }

            auto tuple_type_annotation = std::make_unique<TupleTypeAnnotation>(std::move(elements));
            tuple_type_annotation->span = cursor_.make_span(start_token, end_token);
            return tuple_type_annotation;
        }

        Token name_token = consume_identifier(ValuascriptErrorCode::MissingTypeAnnotation);
        std::vector<std::unique_ptr<TypeAnnotation> > generic_args;

        if (cursor_.match({TokenType::Less})) {
            generic_args = parse_list<std::unique_ptr<TypeAnnotation> >(
                TokenType::Greater,
                std::make_optional(ValuascriptErrorCode::TrailingCommaInGenericArgument),
                ValuascriptErrorCode::ExpectedCommaSeparatorInGenericArgs,
                std::vector<TokenType>{},
                [&]() { return parse_type_annotation(is_at_parent_boundary); },
                is_at_parent_boundary
            );

            if (generic_args.empty()) {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::EmptyGenericTypeAnnotation);
            }

            try {
                cursor_.consume(TokenType::Greater, ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs);
            } catch (const ParseSyncException &) {
                TokenType peek_type = cursor_.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) &&
                    !is_active_closer(peek_type)) {
                    cursor_.advance();
                }
            }
        }

        auto type_ann = std::make_unique<TypeAnnotation>(name_token.lexeme, std::move(generic_args));
        type_ann->span = cursor_.make_span(start_token, cursor_.previous());
        return type_ann;
    }

    std::unique_ptr<TypeAliasDefinition> Parser::parse_type_alias_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Typealias, ValuascriptErrorCode::ExpectedTypeAliasToken);
        Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedTypeAliasName);

        cursor_.consume(TokenType::Assign, ValuascriptErrorCode::ExpectedAssignAfterTypeAliasName);

        bool next_is_newline_stmt = cursor_.peek().line > cursor_.previous().line &&
                                    (TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                                     TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type));

        if (cursor_.is_at_end() || next_is_newline_stmt) {
            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingTypeAnnotation, false);
        }

        if (is_reserved_keyword(cursor_.peek())) {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
            cursor_.advance();
            throw ParseSyncException();
        }

        if (!TokenTraits::is_identifier_start(cursor_.peek()) && !cursor_.check(TokenType::LeftParen)) {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingTypeAnnotation, true);
            cursor_.advance();
            throw ParseSyncException();
        }

        auto target_type = parse_type_annotation();

        if (target_type) {
            verify_statement_end();
        }

        auto type_alias = std::make_unique<TypeAliasDefinition>(std::move(modifiers), name_token.lexeme,
                                                                std::move(target_type));
        type_alias->span = cursor_.make_span(start_token, cursor_.previous());
        return type_alias;
    }
}
