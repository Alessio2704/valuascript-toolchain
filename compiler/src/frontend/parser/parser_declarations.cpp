#include "parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler
{
    std::unique_ptr<ImportStatement> Parser::parse_import_statement()
    {
        const Token& start_token = cursor_.consume(TokenType::Import, ValuascriptErrorCode::ExpectedImportToken);

        Token path(TokenType::String, "<error>", cursor_.peek().line, cursor_.peek().column);
        try
        {
            path = cursor_.consume(TokenType::String, ValuascriptErrorCode::MissingImportPathString);
        }
        catch (const ParseSyncException&)
        {
            synchronize_with({
                .stop_tokens = {TokenType::Comma},
                .stop_at_statement_boundary_respecting_dangling_op = true
            });
        }

        return make_node<ImportStatement>(start_token, path.lexeme);
    }

    std::unique_ptr<Directive> Parser::parse_directive()
    {
        const Token& start_token = cursor_.consume(TokenType::Hash, ValuascriptErrorCode::ExpectedHashToken);

        Token name_token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);

        if (cursor_.is_at_end() || cursor_.peek().line > cursor_.previous().line ||
            is_active_closer(cursor_.peek().type) || is_in_sync_set(cursor_.peek().type))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingDirectiveName, false);
        }
        else
        {
            try
            {
                name_token = consume_identifier(ValuascriptErrorCode::MissingDirectiveName);
            }
            catch (const ParseSyncException&)
            {
                synchronize_with({
                    .stop_at_any_newline = true
                });
            }
        }

        std::string directive_name = name_token.lexeme;

        std::unique_ptr<Expression> value = nullptr;

        if (directive_name != "<error>")
        {
            if (cursor_.match({TokenType::Assign}))
            {
                bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                (cursor_.peek().line > cursor_.previous().line &&
                    cursor_.peek().type == TokenType::Identifier &&
                    cursor_.peek(1).type == TokenType::Assign);

                if (cursor_.is_at_end() || is_pseudo_stmt || cursor_.peek().line > cursor_.previous().line ||
                    is_active_closer(cursor_.peek().type) || is_in_sync_set(cursor_.peek().type))
                {
                    cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals, false);
                }
                else
                {
                    try
                    {
                        value = parse_expression();
                    }
                    catch (const ParseSyncException&)
                    {
                        synchronize_with({
                            .stop_at_any_newline = true
                        });
                    }
                }
            }
            else if (cursor_.peek().line == cursor_.previous().line && TokenTraits::is_expression_start(
                cursor_.peek().type))
            {
                try
                {
                    value = parse_expression();
                }
                catch (const ParseSyncException&)
                {
                    synchronize_with({
                        .stop_at_any_newline = true
                    });
                }
            }

            if (value)
            {
                verify_statement_end();
            }
        }

        return make_node<Directive>(start_token, directive_name, std::move(value));
    }

    std::vector<Modifier> Parser::parse_modifiers(bool is_statement_context)
    {
        std::vector<Modifier> modifiers;

        while (cursor_.match({TokenType::At}))
        {
            const Token& start_token = cursor_.previous();

            try
            {
                Token name_token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
                try
                {
                    name_token = consume_identifier(ValuascriptErrorCode::ExpectedModifierName, is_statement_context);
                }
                catch (const ParseSyncException&)
                {
                    synchronize_with({
                        .stop_tokens = {TokenType::LeftParen, TokenType::At},
                        .stop_at_statement_boundary_respecting_dangling_op = true,
                        .custom_stop_predicate = [](const Token& tok, TokenType next)
                        {
                            TokenType t = tok.type;
                            return t == TokenType::Identifier ||
                                TokenTraits::acts_like_identifier(tok, next) ||
                                t == TokenType::Colon || t == TokenType::Comma || t == TokenType::Assign ||
                                TokenTraits::is_grouping_closer(t) || TokenTraits::is_grouping_opener(t);
                        }
                    });
                }

                std::vector<std::pair<std::string, std::unique_ptr<Expression>>> arguments;

                if (cursor_.match({TokenType::LeftParen}))
                {
                    CloserTracker tracker(*this, TokenType::RightParen);

                    ParameterRuleSpec arg_spec{
                        .allow_modifiers = false,
                        .allow_type = false,
                        .allow_value = true,
                        .require_value = true,
                        .value_separator = TokenType::Colon,
                        .missing_name_err = ValuascriptErrorCode::MissingArgumentNameInModifier,
                        .missing_value_separator_err = ValuascriptErrorCode::MissingColonAfterArgument,
                        .missing_value_err = ValuascriptErrorCode::InvalidExpression,
                        .unexpected_modifier_err = ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration
                    };

                    auto args_gen = parse_list<GenericParameter>(
                        TokenType::RightParen,
                        std::make_optional(ValuascriptErrorCode::TrailingCommaInModifier),
                        std::make_optional(ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInModifier),
                        std::vector<TokenType>{},
                        [this]()
                        {
                            const Token& tok = cursor_.peek();
                            bool is_id_like = tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(
                                tok, cursor_.peek(1).type);
                            return is_id_like && cursor_.peek(1).type == TokenType::Colon;
                        },
                        [&]() { return parse_generic_parameter(arg_spec); }
                    );

                    arguments.reserve(args_gen.size());
                    for (auto& g : args_gen)
                    {
                        arguments.emplace_back(g.name.lexeme, std::move(g.value));
                    }

                    try
                    {
                        cursor_.consume(TokenType::RightParen,
                                        ValuascriptErrorCode::UnmatchedParenthesisAfterModifierArgs);
                    }
                    catch (const ParseSyncException&)
                    {
                        synchronize_and_consume_closer(TokenType::RightParen);

                        Modifier mod;
                        mod.name = name_token.lexeme;
                        mod.arguments = std::move(arguments);
                        mod.span = cursor_.make_span(start_token, cursor_.previous());
                        modifiers.push_back(std::move(mod));

                        if (is_active_closer(cursor_.peek().type) && cursor_.peek().type != TokenType::RightParen)
                        {
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
            }
            catch (const ParseSyncException&)
            {
                if (is_active_closer(cursor_.peek().type))
                {
                    throw;
                }

                synchronize_with({
                    .stop_tokens = {TokenType::At},
                    .stop_at_statement_boundary_respecting_dangling_op = true,
                    .stop_at_currently_tracked_closers = true,
                    .skip_nested_groupings_during_recovery = true,
                    .custom_stop_predicate = [](const Token& tok, TokenType /*next*/)
                    {
                        return TokenTraits::is_grouping_closer(tok.type);
                    }
                });

                if (cursor_.is_at_end() ||
                    TokenTraits::is_grouping_closer(cursor_.peek().type) ||
                    (cursor_.peek().type != TokenType::At && TokenTraits::is_statement_start(
                        cursor_.peek(), cursor_.peek(1).type)))
                {
                    break;
                }
            }
        }

        return modifiers;
    }

    std::unique_ptr<StructDefinition> Parser::parse_struct_definition(std::vector<Modifier> modifiers)
    {
        const Token& start_token = cursor_.consume(TokenType::Struct, ValuascriptErrorCode::ExpectedStructToken);

        Token name_token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
        try
        {
            name_token = consume_identifier(ValuascriptErrorCode::ExpectedStructName);
        }
        catch (const ParseSyncException&)
        {
            synchronize_with({
                .stop_tokens = {TokenType::LeftBrace, TokenType::Comma},
                .stop_at_statement_boundary_respecting_dangling_op = true
            });
        }

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedBraceInStructDefinition);
        CloserTracker tracker(*this, TokenType::RightBrace);

        auto is_at_parent_boundary = [this](size_t offset = 0)
        {
            const Token& tok = cursor_.peek(offset);
            const TokenType next = cursor_.peek(offset + 1).type;
            return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next ==
                TokenType::Colon;
        };

        ParameterRuleSpec field_spec{
            .allow_modifiers = true,
            .allow_type = true,
            .require_type = true,
            .allow_value = false,
            .missing_name_err = ValuascriptErrorCode::ExpectedStructFieldName,
            .missing_type_colon_err = ValuascriptErrorCode::ExpectedColonAfterStructFieldName
        };

        auto fields_gen = parse_list<GenericParameter>(
            TokenType::RightBrace,
            std::nullopt,
            std::make_optional(ValuascriptErrorCode::ExpectedCommaSeparatorInStruct),
            std::vector<TokenType>{},
            [this]()
            {
                const Token& tok = cursor_.peek();
                const TokenType next = cursor_.peek(1).type;

                if (tok.type == TokenType::At) return !is_at_any_declaration();
                if (tok.type == TokenType::Identifier) return true;

                return is_reserved_keyword(tok) && (next == TokenType::Colon);
            },
            [&]() { return parse_generic_parameter(field_spec, is_at_parent_boundary); }
        );

        std::vector<StructField> fields;
        fields.reserve(fields_gen.size());
        for (auto& g : fields_gen)
        {
            fields.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.type), g.span});
        }

        Token end_token = cursor_.previous();
        try
        {
            end_token = cursor_.consume(TokenType::RightBrace, ValuascriptErrorCode::ExpectedRightBraceAfterStructBody);
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBrace);
            end_token = cursor_.previous();
        }

        return make_node_with_span<StructDefinition>(
            cursor_.make_span(start_token, end_token), std::move(modifiers), name_token.lexeme, std::move(fields));
    }

    std::unique_ptr<EnumDefinition> Parser::parse_enum_definition(std::vector<Modifier> modifiers)
    {
        const Token& start_token = cursor_.consume(TokenType::Enum, ValuascriptErrorCode::ExpectedEnumToken);

        Token name_token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
        try
        {
            name_token = consume_identifier(ValuascriptErrorCode::ExpectedEnumName);
        }
        catch (const ParseSyncException&)
        {
            synchronize_with({
                .stop_tokens = {TokenType::Colon, TokenType::LeftBrace, TokenType::Comma},
                .stop_at_statement_boundary_respecting_dangling_op = true
            });
        }

        std::unique_ptr<TypeAnnotation> underlying_type = nullptr;
        if (cursor_.check(TokenType::Colon))
        {
            cursor_.advance();
            try
            {
                underlying_type = parse_type_annotation();
            }
            catch (const ParseSyncException&)
            {
                synchronize_with({
                    .stop_tokens = {TokenType::LeftBrace, TokenType::Comma},
                    .stop_at_statement_boundary_respecting_dangling_op = true
                });
            }
        }
        else if (cursor_.check(TokenType::LeftBrace))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ExpectedColonAfterEnumName);
        }
        else
        {
            cursor_.consume(TokenType::Colon, ValuascriptErrorCode::ExpectedColonAfterEnumName);
        }

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeEnumBody);
        CloserTracker tracker(*this, TokenType::RightBrace);

        ParameterRuleSpec case_spec{
            .allow_modifiers = true,
            .allow_type = false,
            .require_type = false,
            .allow_value = true,
            .require_value = false,
            .value_separator = TokenType::Assign,
            .missing_name_err = ValuascriptErrorCode::ExpectedEnumCaseName,
            .missing_value_separator_err = ValuascriptErrorCode::MissingOperator,
            .missing_value_err = ValuascriptErrorCode::InvalidExpression
        };

        auto cases_gen = parse_list<GenericParameter>(
            TokenType::RightBrace,
            std::nullopt,
            std::make_optional(ValuascriptErrorCode::ExpectedCommaSeparatorInEnum),
            std::vector<TokenType>{},
            [this]()
            {
                const Token& tok = cursor_.peek();
                const Token& next = cursor_.peek(1);

                if (tok.type == TokenType::At) return !is_at_any_declaration();
                if (tok.type == TokenType::At || tok.type == TokenType::Identifier) return true;

                if (is_reserved_keyword(tok))
                {
                    if (TokenTraits::is_top_level_only_declaration(tok.type)) return false;
                    if (tok.type == TokenType::Let) return next.type != TokenType::Identifier;
                    return true;
                }
                return false;
            },
            [&]() { return parse_generic_parameter(case_spec); }
        );

        std::vector<EnumCase> cases;
        cases.reserve(cases_gen.size());
        for (auto& g : cases_gen)
        {
            cases.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.value)});
        }

        Token end_token = cursor_.previous();
        try
        {
            end_token = cursor_.consume(TokenType::RightBrace, ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody);
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBrace);
            end_token = cursor_.previous();
        }

        return make_node_with_span<EnumDefinition>(
            cursor_.make_span(start_token, end_token), std::move(modifiers), name_token.lexeme,
            std::move(underlying_type), std::move(cases));
    }

    std::unique_ptr<FunctionDefinition> Parser::parse_function_definition(std::vector<Modifier> modifiers)
    {
        const Token& start_token = cursor_.consume(TokenType::Func, ValuascriptErrorCode::ExpectedFuncToken);

        Token name(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
        try
        {
            name = consume_identifier(ValuascriptErrorCode::MissingFunctionName);
        }
        catch (const ParseSyncException&)
        {
            synchronize_with({
                .stop_tokens = {TokenType::LeftParen, TokenType::LeftBrace, TokenType::Comma},
                .stop_at_statement_boundary_respecting_dangling_op = true
            });
        }

        cursor_.consume(TokenType::LeftParen, ValuascriptErrorCode::ExpectedLeftParenAfterFunctionName);

        auto is_at_parent_boundary = [this](size_t offset = 0)
        {
            const Token& tok = cursor_.peek(offset);
            const TokenType next = cursor_.peek(offset + 1).type;
            return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next ==
                TokenType::Colon;
        };

        std::vector<FunctionParameter> params;
        {
            CloserTracker param_tracker(*this, TokenType::RightParen);

            ParameterRuleSpec param_spec{
                .allow_modifiers = true,
                .allow_type = true,
                .require_type = true,
                .allow_value = true,
                .require_value = false,
                .value_separator = TokenType::Assign,
                .missing_name_err = ValuascriptErrorCode::MissingParameterName,
                .missing_type_colon_err = ValuascriptErrorCode::MissingColonAfterParameter,
                .missing_value_err = ValuascriptErrorCode::MissingDefaultParameterValue
            };

            auto params_gen = parse_list<GenericParameter>(
                TokenType::RightParen,
                std::make_optional(ValuascriptErrorCode::TrailingComma),
                std::make_optional(ValuascriptErrorCode::ExpectedCommaSeparatorInParameterList),
                std::vector<TokenType>{},
                [this]()
                {
                    const Token& tok = cursor_.peek();
                    return tok.type == TokenType::At || tok.type == TokenType::Identifier ||
                        TokenTraits::acts_like_identifier(tok, cursor_.peek(1).type);
                },
                [&]() { return parse_generic_parameter(param_spec, is_at_parent_boundary); }
            );

            bool seen_default_param = false;
            for (auto& g : params_gen)
            {
                if (g.has_value_separator || g.value)
                {
                    seen_default_param = true;
                }
                else if (seen_default_param)
                {
                    cursor_.report_error_no_panic(g.name, ValuascriptErrorCode::NonDefaultParameterAfterDefault);
                }
                params.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.type), std::move(g.value)});
            }
        }

        cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::ExpectedRightParenAfterParameters);

        std::vector<std::unique_ptr<TypeAnnotation>> return_types;
        if (cursor_.check(TokenType::Arrow))
        {
            cursor_.advance();
            try
            {
                return_types = parse_list<std::unique_ptr<TypeAnnotation>>(
                    TokenType::LeftBrace,
                    std::make_optional(ValuascriptErrorCode::TrailingComma),
                    ValuascriptErrorCode::ExpectedCommaSeparatorInReturnTypeList,
                    std::vector<TokenType>{},
                    [&]() { return parse_type_annotation(); }
                );

                if (return_types.empty())
                {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::MissingTypeAnnotationAfterArrow);
                }
            }
            catch (const ParseSyncException&)
            {
                synchronize_with({
                    .stop_tokens = {TokenType::LeftBrace},
                    .stop_at_statement_boundary_respecting_dangling_op = true
                });
            }
        }
        else if (cursor_.check(TokenType::LeftBrace))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingArrowInFunction);
        }
        else
        {
            cursor_.consume(TokenType::Arrow, ValuascriptErrorCode::MissingArrowInFunction);
        }

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeFunctionBody);

        CloserTracker body_tracker(*this, TokenType::RightBrace);

        std::optional<std::string> docstring = std::nullopt;
        if (cursor_.check(TokenType::DocString))
        {
            docstring = cursor_.advance().lexeme;
        }

        std::vector<std::unique_ptr<Statement>> body;
        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end())
        {
            if (is_at_top_level_declaration() && is_missing_closing_brace())
            {
                break;
            }
            try
            {
                parse_statement_or_declaration(ParseContext::FunctionBody, nullptr, body);
            }
            catch (const ParseSyncException&)
            {
                synchronize_with({
                    .stop_tokens = {TokenType::RightBrace, TokenType::Return},
                    .force_stop_at_statement_boundary_ignoring_dangling_op = true,
                    .stop_at_currently_tracked_closers = false,
                    .stop_at_currently_tracked_sync_tokens = false,
                    .skip_nested_groupings_during_recovery = false
                });
            }
        }

        Token end_token = cursor_.previous();
        try
        {
            end_token = cursor_.consume(TokenType::RightBrace,
                                        ValuascriptErrorCode::ExpectedRightBraceAfterFunctionBody);
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBrace);
            end_token = cursor_.previous();
        }

        return make_node_with_span<FunctionDefinition>(
            cursor_.make_span(start_token, end_token), std::move(modifiers), name.lexeme, std::move(params),
            std::move(return_types), std::move(body), std::move(docstring));
    }

    std::unique_ptr<TypeAnnotation> Parser::parse_type_annotation(const ParentBoundaryPredicate& is_at_parent_boundary)
    {
        const Token& start_token = cursor_.peek();

        if (cursor_.match({TokenType::LeftParen}))
        {
            CloserTracker tracker(*this, TokenType::RightParen);

            auto elements = parse_list<std::unique_ptr<TypeAnnotation>>(
                TokenType::RightParen,
                std::make_optional(ValuascriptErrorCode::SingleElementTuplesNotAllowed),
                ValuascriptErrorCode::ExpectedCommaSeparatorInTupleType,
                std::vector<TokenType>{},
                [&]() { return parse_type_annotation(is_at_parent_boundary); },
                is_at_parent_boundary
            );

            Token end_token = cursor_.previous();
            try
            {
                end_token = cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::UnmatchedParenthesisInTuple);
            }
            catch (const ParseSyncException&)
            {
                TokenType peek_type = cursor_.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) &&
                    !is_active_closer(peek_type))
                {
                    end_token = cursor_.advance();
                }
                else
                {
                    end_token = cursor_.previous();
                }
            }

            return make_node_with_span<TupleTypeAnnotation>(cursor_.make_span(start_token, end_token),
                                                            std::move(elements));
        }

        Token name_token = consume_identifier(ValuascriptErrorCode::MissingTypeAnnotation);
        std::vector<std::unique_ptr<TypeAnnotation>> generic_args;

        if (cursor_.match({TokenType::Less}))
        {
            generic_args = parse_list<std::unique_ptr<TypeAnnotation>>(
                TokenType::Greater,
                std::make_optional(ValuascriptErrorCode::TrailingCommaInGenericArgument),
                ValuascriptErrorCode::ExpectedCommaSeparatorInGenericArgs,
                std::vector<TokenType>{},
                [&]() { return parse_type_annotation(is_at_parent_boundary); },
                is_at_parent_boundary
            );

            if (generic_args.empty())
            {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::EmptyGenericTypeAnnotation);
            }

            try
            {
                cursor_.consume(TokenType::Greater, ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs);
            }
            catch (const ParseSyncException&)
            {
                TokenType peek_type = cursor_.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) &&
                    !is_active_closer(peek_type))
                {
                    cursor_.advance();
                }
            }
        }

        return make_node<TypeAnnotation>(start_token, name_token.lexeme, std::move(generic_args));
    }

    std::unique_ptr<TypeAliasDefinition> Parser::parse_type_alias_definition(std::vector<Modifier> modifiers)
    {
        const Token& start_token = cursor_.consume(TokenType::Typealias, ValuascriptErrorCode::ExpectedTypeAliasToken);

        Token name_token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
        try
        {
            name_token = consume_identifier(ValuascriptErrorCode::ExpectedTypeAliasName);
        }
        catch (const ParseSyncException&)
        {
            synchronize_with({
                .stop_tokens = {TokenType::Assign, TokenType::Comma},
                .stop_at_statement_boundary_respecting_dangling_op = true
            });
        }

        if (cursor_.check(TokenType::Assign))
        {
            cursor_.advance();
        }
        else
        {
            cursor_.consume(TokenType::Assign, ValuascriptErrorCode::ExpectedAssignAfterTypeAliasName);
        }

        bool next_is_newline_stmt = cursor_.peek().line > cursor_.previous().line &&
        (TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
            TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type));

        if (cursor_.is_at_end() || next_is_newline_stmt)
        {
            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingTypeAnnotation, false);
        }

        if (is_reserved_keyword(cursor_.peek()))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
            cursor_.advance();
            throw ParseSyncException();
        }

        if (!TokenTraits::is_identifier_start(cursor_.peek()) && !cursor_.check(TokenType::LeftParen))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingTypeAnnotation, true);
            cursor_.advance();
            throw ParseSyncException();
        }

        auto target_type = parse_type_annotation();

        if (target_type)
        {
            verify_statement_end();
        }

        return make_node<TypeAliasDefinition>(start_token, std::move(modifiers), name_token.lexeme,
                                              std::move(target_type));
    }
}
