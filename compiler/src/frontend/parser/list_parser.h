#pragma once
#include <vector>
#include <optional>
#include <functional>
#include "parser_context.h"
#include "error_recovery.h"

namespace valuascript::compiler
{
    template <typename ElementType>
    class ListParser
    {
    private:
        ParserContext& ctx_;
        TokenType closing_token_ = TokenType::EndOfFile;
        std::optional<ParserErrorCode> trailing_comma_err_ = std::nullopt;
        std::optional<ParserErrorCode> missing_comma_err_ = std::nullopt;
        std::vector<TokenType> recovery_boundaries_ = {};
        std::function<bool()> is_element_start_ = nullptr;
        std::function<bool(int)> is_at_parent_boundary_ = nullptr;

    public:
        explicit ListParser(ParserContext& ctx) : ctx_(ctx)
        {
        }

        ListParser& stop_at(TokenType closing_token)
        {
            closing_token_ = closing_token;
            return *this;
        }

        ListParser& on_trailing_comma(std::optional<ParserErrorCode> err)
        {
            trailing_comma_err_ = err;
            return *this;
        }

        ListParser& on_missing_comma(std::optional<ParserErrorCode> err)
        {
            missing_comma_err_ = err;
            return *this;
        }

        ListParser& with_recovery_boundaries(std::vector<TokenType> boundaries)
        {
            recovery_boundaries_ = std::move(boundaries);
            return *this;
        }

        ListParser& is_element_start(std::function<bool()> predicate)
        {
            is_element_start_ = std::move(predicate);
            return *this;
        }

        ListParser& is_at_parent_boundary(std::function<bool(int)> predicate)
        {
            is_at_parent_boundary_ = std::move(predicate);
            return *this;
        }

        template <typename ElementParser>
        std::vector<ElementType> parse_elements(ElementParser parse_element)
        {
            if (!is_element_start_)
            {
                is_element_start_ = [&]()
                {
                    const Token& tok = ctx_.cursor.peek();
                    return tok.type == TokenType::Identifier ||
                        TokenTraits::acts_like_identifier(tok, ctx_.cursor.peek(1).type) ||
                        tok.type == TokenType::LeftParen;
                };
            }

            std::vector<ElementType> elements;

            auto is_hard_stop = [&](const Token& token, TokenType next)
            {
                if (is_element_start_())
                {
                    if (TokenTraits::is_newline_statement_boundary(ctx_.cursor.previous(), token, next))
                    {
                        if (token.type != TokenType::At || ctx_.is_at_any_declaration()) return true;
                    }
                    return false;
                }
                if (TokenTraits::is_newline_statement_boundary(ctx_.cursor.previous(), token, next)) return true;
                for (TokenType stop : recovery_boundaries_) if (token.type == stop) return true;
                return false;
            };

            while (!ctx_.cursor.check(closing_token_) && !ctx_.cursor.is_at_end())
            {
                if (is_at_parent_boundary_ && is_at_parent_boundary_(0)) break;

                try
                {
                    const Token& tok = ctx_.cursor.peek();
                    TokenType next = ctx_.cursor.peek(1).type;

                    if ((TokenTraits::is_statement_start(tok, next) ||
                        TokenTraits::is_top_level_only_declaration(tok.type)) && !is_element_start_())
                    {
                        if (tok.line > ctx_.cursor.previous().line) break;
                        else
                        {
                            const Token& start_tok = ctx_.cursor.peek();
                            if (ctx_.on_unexpected_statement) ctx_.on_unexpected_statement();
                            ctx_.cursor.report_error_no_panic(ctx_.cursor.make_span(start_tok, ctx_.cursor.previous()),
                                                              ParserErrorCode::TopLevelDeclarationNotAllowedHere);
                            throw ParseSyncException();
                        }
                    }

                    elements.push_back(parse_element());
                    if (is_at_parent_boundary_ && is_at_parent_boundary_(0)) break;

                    if (ctx_.cursor.check(TokenType::Comma))
                    {
                        if (is_at_parent_boundary_ && is_at_parent_boundary_(1)) break;
                        ctx_.cursor.advance();
                        if (ctx_.cursor.check(closing_token_) && trailing_comma_err_)
                            ctx_.cursor.report_error(ctx_.cursor.previous(), *trailing_comma_err_);
                    }
                    else if (!ctx_.cursor.check(closing_token_))
                    {
                        bool is_boundary = TokenTraits::is_newline_statement_boundary(
                            ctx_.cursor.previous(), ctx_.cursor.peek(), ctx_.cursor.peek(1).type);
                        if (is_boundary && ctx_.cursor.peek().type == TokenType::At)
                        {
                            if (!ctx_.is_at_any_declaration()) is_boundary = false;
                        }
                        if (is_element_start_() && !is_boundary)
                        {
                            if (missing_comma_err_)
                                ctx_.cursor.report_error_no_panic(
                                    ctx_.cursor.peek(), *missing_comma_err_);
                        }
                        else break;
                    }
                }
                catch (const ParseSyncException&)
                {
                    ErrorRecovery::synchronize_with(
                        ctx_, {
                            .stop_tokens = {TokenType::Comma, closing_token_},
                            .options = DefaultRecoveryOptions |
                            RecoveryOptions::StopAtBoundaryRespectingDanglingOp |
                            RecoveryOptions::IgnoreStandaloneModifiersAsBoundaries,
                            .custom_stop_predicate = [&](const Token&, TokenType)
                            {
                                return is_at_parent_boundary_ && is_at_parent_boundary_(0);
                            }
                        });
                    if (ctx_.cursor.check(TokenType::Comma))
                    {
                        if (is_at_parent_boundary_ && is_at_parent_boundary_(1)) break;
                        ctx_.cursor.advance();
                    }
                }
                if (ctx_.cursor.peek().type != closing_token_ && is_hard_stop(
                    ctx_.cursor.peek(), ctx_.cursor.peek(1).type))
                    break;
            }
            return elements;
        }
    };
}
