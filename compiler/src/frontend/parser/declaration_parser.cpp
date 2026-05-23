#include "declaration_parser.h"
#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include "ast_factory.h"
#include "list_parser.h"
#include "error_recovery.h"
#include "declaration_rules.h"

namespace valuascript::compiler
{
    using E = ParserErrorCode;

    DeclarationParser::DeclarationParser(Parser& p) : parser(p), ctx(p.ctx), cursor(p.ctx.cursor)
    {
    }

    ImportPtr DeclarationParser::parse_import_statement()
    {
        const Token& start = cursor.consume(TokenType::Import, E::ExpectedImportToken);
        Token path = ErrorRecovery::try_consume(
            ctx, TokenType::String, E::MissingImportPathString,
            RecoveryConfig::StopAtBoundary({TokenType::Comma})
        );
        return AstFactory::make_node<ImportStatement>(cursor, start, path.lexeme);
    }

    DirectivePtr DeclarationParser::parse_directive()
    {
        const Token& start = cursor.consume(TokenType::Hash, E::ExpectedHashToken);
        Token name_token(TokenType::Identifier, "<error>", cursor.peek().line, cursor.peek().column);

        if (cursor.is_at_end() || cursor.peek().line > cursor.previous().line || ctx.
            is_active_closer(cursor.peek().type) || ctx.is_in_sync_set(cursor.peek().type))
        {
            cursor.report_error_no_panic(cursor.peek(), E::MissingDirectiveName, false);
        }
        else
        {
            name_token = ErrorRecovery::try_consume_identifier(ctx, E::MissingDirectiveName,
                                                               RecoveryConfig::StopAtNewline());
        }

        std::string directive_name = name_token.lexeme;
        ExprPtr value = nullptr;

        if (directive_name != "<error>")
        {
            if (cursor.match({TokenType::Assign}))
            {
                bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) || (cursor.
                    peek().line > cursor.previous().line && cursor.peek().type == TokenType::Identifier && cursor.
                    peek(1).type == TokenType::Assign);
                if (cursor.is_at_end() || is_pseudo_stmt || cursor.peek().line > cursor.previous().line || ctx.
                    is_active_closer(cursor.peek().type) || ctx.is_in_sync_set(cursor.peek().type))
                {
                    cursor.report_error_no_panic(cursor.peek(), E::MissingValueAfterEquals, false);
                }
                else
                {
                    value = ErrorRecovery::try_parse<ExprPtr>(
                        ctx, [&]() { return parser.parse_expression(); }, RecoveryConfig::StopAtNewline()
                    );
                }
            }
            else if (cursor.peek().line == cursor.previous().line && TokenTraits::is_expression_start(
                cursor.peek().type))
            {
                value = ErrorRecovery::try_parse<ExprPtr>(
                    ctx, [&]() { return parser.parse_expression(); }, RecoveryConfig::StopAtNewline());
            }

            if (value) parser.verify_statement_end();
        }
        return AstFactory::make_node<Directive>(cursor, start, directive_name, std::move(value));
    }

    std::vector<Modifier> DeclarationParser::parse_modifiers(bool is_statement_context)
    {
        std::vector<Modifier> modifiers;
        while (cursor.match({TokenType::At}))
        {
            const Token& start_token = cursor.previous();
            try
            {
                RecoveryConfig config;
                config.stop_tokens = {TokenType::LeftParen, TokenType::At};
                config.options = DefaultRecoveryOptions | RecoveryOptions::StopAtBoundaryRespectingDanglingOp;
                config.custom_stop_predicate = [](const Token& tok, TokenType next)
                {
                    return tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next) || tok.type
                        == TokenType::Colon || tok.type == TokenType::Comma || tok.type == TokenType::Assign ||
                        TokenTraits::is_grouping_closer(tok.type) || TokenTraits::is_grouping_opener(tok.type);
                };

                Token name_token = ErrorRecovery::try_consume_identifier(
                    ctx, E::ExpectedModifierName, config, is_statement_context);
                std::vector<std::pair<std::string, ExprPtr>> arguments;

                if (cursor.match({TokenType::LeftParen}))
                {
                    CloserTracker tracker(ctx, TokenType::RightParen);
                    ParameterRuleSpec arg_spec{
                        .allow_value = true, .require_value = true, .value_separator = TokenType::Colon,
                        .missing_name_err = E::MissingArgumentNameInModifier,
                        .missing_value_separator_err = E::MissingColonAfterArgument,
                        .missing_value_err = E::InvalidExpression
                    };

                    auto args_gen = ListParser<GenericParameter>(ctx)
                                    .stop_at(TokenType::RightParen)
                                    .on_trailing_comma(E::TrailingCommaInModifier)
                                    .on_missing_comma(E::MissingCommaSeparatorForArgumentsInModifier)
                                    .is_element_start([this]()
                                    {
                                        const Token& tok = cursor.peek();
                                        return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(
                                            tok, cursor.peek(1).type)) && cursor.peek(1).type == TokenType::Colon;
                                    })
                                    .parse_elements([&]() { return parse_generic_parameter(arg_spec); });

                    for (auto& g : args_gen) arguments.emplace_back(g.name.lexeme, std::move(g.value));

                    try { cursor.consume(TokenType::RightParen, E::UnmatchedParenthesisAfterModifierArgs); }
                    catch (const ParseSyncException&)
                    {
                        ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightParen);
                        Modifier mod;
                        mod.name = name_token.lexeme;
                        mod.arguments = std::move(arguments);
                        mod.span = cursor.make_span(start_token, cursor.previous());
                        modifiers.push_back(std::move(mod));
                        if (ctx.is_active_closer(cursor.peek().type) && cursor.peek().type != TokenType::RightParen)
                            throw ParseSyncException();
                        continue;
                    }
                }

                Modifier mod;
                mod.name = name_token.lexeme;
                mod.arguments = std::move(arguments);
                mod.span = cursor.make_span(start_token, cursor.previous());
                modifiers.push_back(std::move(mod));
            }
            catch (const ParseSyncException&)
            {
                if (ctx.is_active_closer(cursor.peek().type)) throw;

                RecoveryConfig sync_config;
                sync_config.stop_tokens = {TokenType::At};
                sync_config.options = DefaultRecoveryOptions | RecoveryOptions::StopAtBoundaryRespectingDanglingOp;
                sync_config.custom_stop_predicate = [](const Token& tok, TokenType)
                {
                    return TokenTraits::is_grouping_closer(tok.type);
                };
                ErrorRecovery::synchronize_with(ctx, sync_config);

                if (cursor.is_at_end() || TokenTraits::is_grouping_closer(cursor.peek().type) || (cursor.peek().type !=
                    TokenType::At && TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type)))
                    break;
            }
        }
        return modifiers;
    }

    StructDefPtr DeclarationParser::parse_struct_definition(std::vector<Modifier> modifiers)
    {
        const Token& start = cursor.consume(TokenType::Struct, E::ExpectedStructToken);
        Token name = ErrorRecovery::try_consume_identifier(
            ctx, E::ExpectedStructName, RecoveryConfig::StopAtBoundary({TokenType::LeftBrace, TokenType::Comma}));

        cursor.consume(TokenType::LeftBrace, E::ExpectedBraceInStructDefinition);
        CloserTracker tracker(ctx, TokenType::RightBrace);

        auto is_at_parent_boundary = [this](size_t offset = 0)
        {
            const Token& tok = cursor.peek(offset);
            const TokenType next = cursor.peek(offset + 1).type;
            return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next ==
                TokenType::Colon;
        };

        ParameterRuleSpec field_spec{
            .allow_modifiers = true, .allow_type = true, .require_type = true,
            .missing_name_err = E::ExpectedStructFieldName,
            .missing_type_colon_err = E::ExpectedColonAfterStructFieldName
        };

        auto fields_gen = ListParser<GenericParameter>(ctx)
                          .stop_at(TokenType::RightBrace)
                          .on_missing_comma(E::ExpectedCommaSeparatorInStruct)
                          .is_element_start([this]()
                          {
                              const Token& tok = cursor.peek();
                              if (tok.type == TokenType::At) return !ctx.is_at_any_declaration();
                              if (tok.type == TokenType::Identifier) return true;
                              return is_reserved_keyword(tok) && (cursor.peek(1).type == TokenType::Colon);
                          })
                          .parse_elements([&]() { return parse_generic_parameter(field_spec, is_at_parent_boundary); });

        std::vector<StructField> fields;
        fields.reserve(fields_gen.size());
        for (auto& g : fields_gen) fields.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.type), g.span});

        Token end_token = cursor.previous();
        try { end_token = cursor.consume(TokenType::RightBrace, E::ExpectedRightBraceAfterStructBody); }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBrace);
            end_token = cursor.previous();
        }

        return AstFactory::make_node_with_span<StructDefinition>(cursor.make_span(start, end_token),
                                                                 std::move(modifiers), name.lexeme, std::move(fields));
    }

    EnumDefPtr DeclarationParser::parse_enum_definition(std::vector<Modifier> modifiers)
    {
        const Token& start = cursor.consume(TokenType::Enum, E::ExpectedEnumToken);
        Token name = ErrorRecovery::try_consume_identifier(
            ctx, E::ExpectedEnumName, RecoveryConfig::StopAtBoundary({
                TokenType::Colon, TokenType::LeftBrace, TokenType::Comma
            }));

        TypeAnnPtr underlying_type = nullptr;
        if (cursor.check(TokenType::Colon))
        {
            cursor.advance();
            underlying_type = ErrorRecovery::try_parse<TypeAnnPtr>(
                ctx, [&]() { return parser.parse_type_annotation(); },
                RecoveryConfig::StopAtBoundary({TokenType::LeftBrace, TokenType::Comma}));
        }
        else if (cursor.check(TokenType::LeftBrace))
        {
            cursor.report_error_no_panic(cursor.peek(), E::ExpectedColonAfterEnumName);
        }
        else { cursor.consume(TokenType::Colon, E::ExpectedColonAfterEnumName); }

        cursor.consume(TokenType::LeftBrace, E::ExpectedLeftBraceBeforeEnumBody);
        CloserTracker tracker(ctx, TokenType::RightBrace);

        ParameterRuleSpec case_spec{
            .allow_modifiers = true, .allow_value = true, .value_separator = TokenType::Assign,
            .missing_name_err = E::ExpectedEnumCaseName, .missing_value_separator_err = E::MissingOperator,
            .missing_value_err = E::InvalidExpression
        };

        auto cases_gen = ListParser<GenericParameter>(ctx)
                         .stop_at(TokenType::RightBrace)
                         .on_missing_comma(E::ExpectedCommaSeparatorInEnum)
                         .is_element_start([this]()
                         {
                             const Token& tok = cursor.peek();
                             const Token& next = cursor.peek(1);
                             if (tok.type == TokenType::At) return !ctx.is_at_any_declaration();
                             if (tok.type == TokenType::At || tok.type == TokenType::Identifier) return true;
                             if (is_reserved_keyword(tok))
                             {
                                 if (TokenTraits::is_top_level_only_declaration(tok.type)) return false;
                                 if (tok.type == TokenType::Let) return next.type != TokenType::Identifier;
                                 return true;
                             }
                             return false;
                         })
                         .parse_elements([&]() { return parse_generic_parameter(case_spec); });

        std::vector<EnumCase> cases;
        cases.reserve(cases_gen.size());
        for (auto& g : cases_gen) cases.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.value)});

        Token end_token = cursor.previous();
        try { end_token = cursor.consume(TokenType::RightBrace, E::ExpectedRightBraceAfterEnumBody); }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBrace);
            end_token = cursor.previous();
        }

        return AstFactory::make_node_with_span<EnumDefinition>(cursor.make_span(start, end_token), std::move(modifiers),
                                                               name.lexeme, std::move(underlying_type),
                                                               std::move(cases));
    }

    FuncDefPtr DeclarationParser::parse_function_definition(std::vector<Modifier> modifiers)
    {
        const Token& start = cursor.consume(TokenType::Func, E::ExpectedFuncToken);
        Token name = ErrorRecovery::try_consume_identifier(
            ctx, E::MissingFunctionName, RecoveryConfig::StopAtBoundary({
                TokenType::LeftParen, TokenType::LeftBrace, TokenType::Comma
            }));

        cursor.consume(TokenType::LeftParen, E::ExpectedLeftParenAfterFunctionName);

        auto is_at_parent_boundary = [this](size_t offset = 0)
        {
            const Token& tok = cursor.peek(offset);
            const TokenType next = cursor.peek(offset + 1).type;
            return (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next ==
                TokenType::Colon;
        };

        std::vector<FunctionParameter> params;
        {
            CloserTracker param_tracker(ctx, TokenType::RightParen);
            ParameterRuleSpec param_spec{
                .allow_modifiers = true, .allow_type = true, .require_type = true, .allow_value = true,
                .require_value = false, .value_separator = TokenType::Assign,
                .missing_name_err = E::MissingParameterName,
                .missing_type_colon_err = E::MissingColonAfterParameter,
                .missing_value_err = E::MissingDefaultParameterValue
            };

            auto params_gen = ListParser<GenericParameter>(ctx)
                              .stop_at(TokenType::RightParen)
                              .on_trailing_comma(E::TrailingComma)
                              .on_missing_comma(E::ExpectedCommaSeparatorInParameterList)
                              .is_element_start([this]()
                              {
                                  const Token& tok = cursor.peek();
                                  return tok.type == TokenType::At || tok.type == TokenType::Identifier ||
                                      TokenTraits::acts_like_identifier(tok, cursor.peek(1).type);
                              })
                              .parse_elements([&]()
                              {
                                  return parse_generic_parameter(param_spec, is_at_parent_boundary);
                              });

            bool seen_default_param = false;
            params.reserve(params_gen.size());
            for (auto& g : params_gen)
            {
                if (g.has_value_separator || g.value) seen_default_param = true;
                else if (seen_default_param) cursor.report_error_no_panic(g.name, E::NonDefaultParameterAfterDefault);
                params.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.type), std::move(g.value)});
            }
        }

        cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterParameters);

        std::vector<TypeAnnPtr> return_types;
        if (cursor.check(TokenType::Arrow))
        {
            cursor.advance();
            ErrorRecovery::attempt_parse_void(
                ctx, [&]()
                {
                    return_types = ListParser<TypeAnnPtr>(ctx)
                                   .stop_at(TokenType::LeftBrace)
                                   .on_trailing_comma(E::TrailingComma)
                                   .on_missing_comma(
                                       E::ExpectedCommaSeparatorInReturnTypeList)
                                   .parse_elements([&]()
                                   {
                                       return parser.parse_type_annotation();
                                   });

                    if (return_types.empty())
                        cursor.report_error_no_panic(
                            cursor.peek(), E::MissingTypeAnnotationAfterArrow);
                }, RecoveryConfig::StopAtBoundary({TokenType::LeftBrace})
            );
        }
        else if (cursor.check(TokenType::LeftBrace))
        {
            cursor.report_error_no_panic(cursor.peek(), E::MissingArrowInFunction);
        }
        else { cursor.consume(TokenType::Arrow, E::MissingArrowInFunction); }

        cursor.consume(TokenType::LeftBrace, E::ExpectedLeftBraceBeforeFunctionBody);
        CloserTracker body_tracker(ctx, TokenType::RightBrace);

        std::optional<std::string> docstring = std::nullopt;
        if (cursor.check(TokenType::DocString)) docstring = cursor.advance().lexeme;

        std::vector<StmtPtr> body;
        while (!cursor.check(TokenType::RightBrace) && !cursor.is_at_end())
        {
            if (ctx.is_at_top_level_declaration() && ctx.is_missing_closing_brace()) break;
            RecoveryConfig body_config;
            body_config.stop_tokens = {TokenType::RightBrace, TokenType::Return};
            body_config.options = RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp;
            ErrorRecovery::attempt_parse_void(
                ctx, [&]() { parser.parse_statement_or_declaration(ParseContextType::FunctionBody, nullptr, body); },
                body_config);
        }

        Token end_token = cursor.previous();
        try { end_token = cursor.consume(TokenType::RightBrace, E::ExpectedRightBraceAfterFunctionBody); }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBrace);
            end_token = cursor.previous();
        }

        return AstFactory::make_node_with_span<FunctionDefinition>(
            cursor.make_span(start, end_token), std::move(modifiers), name.lexeme, std::move(params),
            std::move(return_types), std::move(body), std::move(docstring)
        );
    }

    TypeAliasPtr DeclarationParser::parse_type_alias_definition(std::vector<Modifier> modifiers)
    {
        const Token& start = cursor.consume(TokenType::Typealias, E::ExpectedTypeAliasToken);
        Token name = ErrorRecovery::try_consume_identifier(
            ctx, E::ExpectedTypeAliasName, RecoveryConfig::StopAtBoundary({TokenType::Assign, TokenType::Comma}));

        if (cursor.check(TokenType::Assign)) cursor.advance();
        else cursor.consume(TokenType::Assign, E::ExpectedAssignAfterTypeAliasName);

        bool next_is_newline_stmt = cursor.peek().line > cursor.previous().line && (
            TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
            TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type));

        if (cursor.is_at_end() || next_is_newline_stmt)
        {
            cursor.report_error_no_panic(cursor.peek(), E::MissingTypeAnnotation, false);
            return AstFactory::make_node<
                TypeAliasDefinition>(cursor, start, std::move(modifiers), name.lexeme, nullptr);
        }

        if (is_reserved_keyword(cursor.peek()))
        {
            cursor.report_error_no_panic(cursor.peek(), E::ReservedKeywordAsIdentifier, true);
            cursor.advance();
            return AstFactory::make_node<
                TypeAliasDefinition>(cursor, start, std::move(modifiers), name.lexeme, nullptr);
        }

        if (!TokenTraits::is_identifier_start(cursor.peek()) && !cursor.check(TokenType::LeftParen))
        {
            cursor.report_error_no_panic(cursor.peek(), E::MissingTypeAnnotation, true);
            cursor.advance();
            return AstFactory::make_node<
                TypeAliasDefinition>(cursor, start, std::move(modifiers), name.lexeme, nullptr);
        }

        auto target_type = ErrorRecovery::attempt_parse<TypeAnnPtr>(
            ctx, [&]() { return parser.parse_type_annotation(); },
            RecoveryConfig::StopAtNewline(),
            nullptr
        );

        if (target_type) parser.verify_statement_end();
        return AstFactory::make_node<TypeAliasDefinition>(cursor, start, std::move(modifiers), name.lexeme,
                                                          std::move(target_type));
    }

    GenericParameter DeclarationParser::parse_generic_parameter(const ParameterRuleSpec& spec,
                                                                const ParentBoundaryPredicate& is_at_parent_boundary)
    {
        GenericParameter result;
        const Token& start = cursor.peek();

        if (spec.allow_modifiers || cursor.check(TokenType::At))
        {
            auto mods = parse_modifiers();
            if (!mods.empty())
            {
                if (!spec.allow_modifiers)
                {
                    SourceSpan span = cursor.combine_spans(mods.front().span, mods.back().span);
                    cursor.report_error_no_panic(span, spec.unexpected_modifier_err);
                }
                else result.modifiers = std::move(mods);
            }
        }

        bool name_failed = false;
        result.name = ErrorRecovery::try_consume_identifier(
            ctx, spec.missing_name_err, RecoveryConfig::StopAtBoundary({
                TokenType::Colon, spec.value_separator, TokenType::Comma
            }),
            false, false, &name_failed);

        if (spec.allow_type)
        {
            if (cursor.check(TokenType::Colon))
            {
                cursor.advance();
                result.type = ErrorRecovery::try_parse<TypeAnnPtr>(
                    ctx, [&]() { return parser.parse_type_annotation(is_at_parent_boundary); },
                    RecoveryConfig::StopAtBoundary({spec.value_separator, TokenType::Comma})
                );
            }
            else if (spec.require_type && !name_failed)
            {
                if (!cursor.check(spec.value_separator) && !cursor.check(TokenType::Comma) && !ctx.is_active_closer(
                    cursor.peek().type))
                    cursor.consume(TokenType::Colon, spec.missing_type_colon_err);
                else cursor.report_error_no_panic(cursor.peek(), spec.missing_type_colon_err);
            }
        }

        if (spec.allow_value)
        {
            bool has_sep = false;
            if (cursor.match({spec.value_separator}))
            {
                has_sep = true;
                result.has_value_separator = true;
            }
            else if (spec.require_value && !name_failed)
            {
                if (!cursor.check(TokenType::Comma) && !ctx.is_active_closer(cursor.peek().type))
                    cursor.consume(spec.value_separator, spec.missing_value_separator_err);
                else cursor.report_error_no_panic(cursor.peek(), spec.missing_value_separator_err);
            }

            if (has_sep || spec.require_value)
            {
                if (cursor.check(TokenType::Comma) || ctx.is_active_closer(cursor.peek().type))
                {
                    if (has_sep) cursor.report_error_no_panic(cursor.peek(), spec.missing_value_err);
                }
                else
                {
                    ErrorRecovery::attempt_parse_void(
                        ctx,
                        [&]()
                        {
                            result.value = parser.parse_expression();
                            if ((TokenTraits::is_expression_start(cursor.peek().type) ||
                                    TokenTraits::is_binary_operator(cursor.peek().type))
                                &&
                                cursor.peek(1).type != spec.value_separator && cursor.
                                                                               peek(1).type != TokenType::Colon)
                            {
                                if (!TokenTraits::is_newline_statement_boundary(
                                    cursor.previous(), cursor.peek(),
                                    cursor.peek(1).type))
                                    cursor.report_error(
                                        cursor.peek(), E::MissingOperator);
                            }
                        }, RecoveryConfig::StopAtBoundary({TokenType::Comma})
                    );
                }
            }
        }

        result.span = cursor.make_span(start, cursor.previous());
        return result;
    }
}
