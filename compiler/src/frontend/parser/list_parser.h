#pragma once
#include <vector>
#include <optional>
#include <functional>
#include "parser_context.h"
#include "error_recovery.h"

namespace valuascript::compiler
{
    class ListParser
    {
    public:
        template <typename ElementType, typename IsElementStart, typename ElementParser>
        static std::vector<ElementType> parse_list(
            ParserContext& ctx,
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const std::optional<ValuascriptErrorCode> missing_comma_err,
            const std::vector<TokenType>& recovery_boundaries,
            IsElementStart is_element_start,
            ElementParser parse_element,
            const std::function<bool(int)>& is_at_parent_boundary = nullptr)
        {
            std::vector<ElementType> elements;

            auto is_hard_stop = [&](const Token& token, TokenType next)
            {
                if (is_element_start())
                {
                    if (TokenTraits::is_newline_statement_boundary(ctx.cursor.previous(), token, next))
                    {
                        if (token.type != TokenType::At || ctx.is_at_any_declaration()) return true;
                    }
                    return false;
                }
                if (TokenTraits::is_newline_statement_boundary(ctx.cursor.previous(), token, next)) return true;
                for (TokenType stop : recovery_boundaries) if (token.type == stop) return true;
                return false;
            };

            while (!ctx.cursor.check(closing_token) && !ctx.cursor.is_at_end())
            {
                if (is_at_parent_boundary && is_at_parent_boundary(0)) break;

                try
                {
                    const Token& tok = ctx.cursor.peek();
                    TokenType next = ctx.cursor.peek(1).type;

                    if ((TokenTraits::is_statement_start(tok, next) ||
                        TokenTraits::is_top_level_only_declaration(tok.type)) && !is_element_start())
                    {
                        if (tok.line > ctx.cursor.previous().line) break;
                        else
                        {
                            const Token& start_tok = ctx.cursor.peek();
                            if (ctx.on_unexpected_statement) ctx.on_unexpected_statement();
                            ctx.cursor.report_error_no_panic(ctx.cursor.make_span(start_tok, ctx.cursor.previous()),
                                                             ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
                            throw ParseSyncException();
                        }
                    }

                    elements.push_back(parse_element());
                    if (is_at_parent_boundary && is_at_parent_boundary(0)) break;

                    if (ctx.cursor.check(TokenType::Comma))
                    {
                        if (is_at_parent_boundary && is_at_parent_boundary(1)) break;
                        ctx.cursor.advance();
                        if (ctx.cursor.check(closing_token) && trailing_comma_err)
                            ctx.cursor.report_error(ctx.cursor.previous(), *trailing_comma_err);
                    }
                    else if (!ctx.cursor.check(closing_token))
                    {
                        bool is_boundary = TokenTraits::is_newline_statement_boundary(
                            ctx.cursor.previous(), ctx.cursor.peek(), ctx.cursor.peek(1).type);
                        if (is_boundary && ctx.cursor.peek().type == TokenType::At)
                        {
                            if (!ctx.is_at_any_declaration()) is_boundary = false;
                        }
                        if (is_element_start() && !is_boundary)
                        {
                            if (missing_comma_err) ctx.cursor.report_error_no_panic(
                                ctx.cursor.peek(), *missing_comma_err);
                        }
                        else break;
                    }
                }
                catch (const ParseSyncException&)
                {
                    ErrorRecovery::synchronize_with(ctx, {
                                                        .stop_tokens = {TokenType::Comma, closing_token},
                                                        .options = DefaultRecoveryOptions |
                                                        RecoveryOptions::StopAtBoundaryRespectingDanglingOp |
                                                        RecoveryOptions::IgnoreStandaloneModifiersAsBoundaries,
                                                        .custom_stop_predicate = [&](const Token&, TokenType)
                                                        {
                                                            return is_at_parent_boundary && is_at_parent_boundary(0);
                                                        }
                                                    });
                    if (ctx.cursor.check(TokenType::Comma))
                    {
                        if (is_at_parent_boundary && is_at_parent_boundary(1)) break;
                        ctx.cursor.advance();
                    }
                }
                if (ctx.cursor.peek().type != closing_token && is_hard_stop(ctx.cursor.peek(), ctx.cursor.peek(1).type))
                    break;
            }
            return elements;
        }

        template <typename ElementType, typename ElementParser>
        static std::vector<ElementType> parse_list(
            ParserContext& ctx,
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const ValuascriptErrorCode missing_comma_err,
            const std::vector<TokenType>& recovery_boundaries,
            ElementParser parse_element,
            const std::function<bool(int)>& is_at_parent_boundary = nullptr)
        {
            return parse_list<ElementType>(ctx, closing_token, trailing_comma_err,
                                           std::make_optional(missing_comma_err), recovery_boundaries,
                                           [&]()
                                           {
                                               const Token& tok = ctx.cursor.peek();
                                               return tok.type == TokenType::Identifier ||
                                                   TokenTraits::acts_like_identifier(tok, ctx.cursor.peek(1).type) ||
                                                   tok.type == TokenType::LeftParen;
                                           }, parse_element, is_at_parent_boundary);
        }
    };
}
