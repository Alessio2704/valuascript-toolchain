#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <utility>

#include "token_cursor.h"
#include "token_traits.h"
#include "ast.h"

namespace valuascript::compiler
{
    enum class RecoveryOptions : uint32_t
    {
        None = 0,
        StopAtBoundaryRespectingDanglingOp = 1 << 0,
        ForceStopAtBoundaryIgnoringDanglingOp = 1 << 1,
        StopAtNewline = 1 << 2,
        StopAtTrackedClosers = 1 << 3,
        StopAtTrackedSyncTokens = 1 << 4,
        SkipNestedGroupings = 1 << 5,
        IgnoreStandaloneModifiersAsBoundaries = 1 << 6,
        StopEarlyIfUnbalancedBlocks = 1 << 7
    };

    constexpr RecoveryOptions operator|(RecoveryOptions a, RecoveryOptions b)
    {
        return static_cast<RecoveryOptions>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr RecoveryOptions operator&(RecoveryOptions a, RecoveryOptions b)
    {
        return static_cast<RecoveryOptions>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    constexpr RecoveryOptions operator~(RecoveryOptions a)
    {
        return static_cast<RecoveryOptions>(~static_cast<uint32_t>(a));
    }

    constexpr RecoveryOptions DefaultRecoveryOptions =
        RecoveryOptions::StopAtTrackedClosers |
        RecoveryOptions::StopAtTrackedSyncTokens |
        RecoveryOptions::SkipNestedGroupings;

    struct RecoveryConfig
    {
        std::vector<TokenType> stop_tokens = {};
        RecoveryOptions options = DefaultRecoveryOptions;
        std::function<bool(const Token& tok, TokenType next)> custom_stop_predicate = nullptr;

        [[nodiscard]] bool has(RecoveryOptions opt) const
        {
            return (options & opt) != RecoveryOptions::None;
        }

        static RecoveryConfig StopAtBoundary(std::vector<TokenType> extra_stops = {})
        {
            return {
                std::move(extra_stops),
                DefaultRecoveryOptions | RecoveryOptions::StopAtBoundaryRespectingDanglingOp,
                nullptr
            };
        }

        static RecoveryConfig ForceStopAtBoundary(std::vector<TokenType> extra_stops = {})
        {
            return {
                std::move(extra_stops),
                DefaultRecoveryOptions | RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp,
                nullptr
            };
        }

        static RecoveryConfig StopAtNewline()
        {
            return {
                {},
                DefaultRecoveryOptions | RecoveryOptions::StopAtNewline,
                nullptr
            };
        }
    };

    struct ParameterRuleSpec
    {
        bool allow_modifiers = false;
        bool allow_type = false;
        bool require_type = false;
        bool allow_value = false;
        bool require_value = false;
        TokenType value_separator = TokenType::Assign;

        ValuascriptErrorCode missing_name_err = ValuascriptErrorCode::ExpectedIdentifier;
        ValuascriptErrorCode missing_type_colon_err = ValuascriptErrorCode::ExpectedColon;
        ValuascriptErrorCode missing_value_separator_err = ValuascriptErrorCode::MissingOperator;
        ValuascriptErrorCode missing_value_err = ValuascriptErrorCode::InvalidExpression;
        ValuascriptErrorCode unexpected_modifier_err = ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration;
    };

    struct GenericParameter
    {
        std::vector<Modifier> modifiers;
        Token name{TokenType::Identifier, "<error>", 0, 0};
        std::unique_ptr<TypeAnnotation> type;
        std::unique_ptr<Expression> value;
        bool has_value_separator = false;
        SourceSpan span;
    };

    class ParserContext
    {
    public:
        TokenCursor cursor;
        std::vector<TokenType> active_closers;
        std::vector<TokenType> sync_set;
        std::function<void()> on_unexpected_statement;

        explicit ParserContext(TokenCursor c);

        struct CloserTracker
        {
            ParserContext& ctx;

            CloserTracker(ParserContext& c, TokenType t) : ctx(c)
            {
                ctx.active_closers.push_back(t);
            }

            ~CloserTracker()
            {
                ctx.active_closers.pop_back();
            }
        };

        struct SyncSetTracker
        {
            ParserContext& ctx;
            size_t previous_size;

            SyncSetTracker(ParserContext& c, const std::vector<TokenType>& tokens) : ctx(c)
            {
                previous_size = ctx.sync_set.size();
                ctx.sync_set.insert(ctx.sync_set.end(), tokens.begin(), tokens.end());
            }

            ~SyncSetTracker()
            {
                ctx.sync_set.resize(previous_size);
            }
        };

        bool is_active_closer(TokenType type) const;
        bool is_in_sync_set(TokenType type) const;

        using SyncPredicate = std::function<bool(TokenType type, int nesting_depth)>;
        void recover(const SyncPredicate& stop_condition);
        void synchronize_with(const RecoveryConfig& config);
        void synchronize_to_closer(TokenType closing_token);
        void synchronize_and_consume_closer(TokenType expected_closer);

        const Token& consume_identifier(ValuascriptErrorCode fallback_err, bool allow_top_level_keywords = true,
                                        bool check_statement_boundary = false);
        TokenType peek_past_modifiers() const;
        bool is_at_top_level_declaration() const;
        bool is_at_any_declaration() const;
        bool is_missing_closing_brace() const;
        void reject_modifiers(const std::vector<Modifier>& modifiers,
                              ValuascriptErrorCode error_code =
                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration) const;

        template <typename ReturnType, typename ParseFunc>
        ReturnType attempt_parse(ParseFunc parse_func, const RecoveryConfig& config, ReturnType fallback_value,
                                 bool* out_failed = nullptr)
        {
            try
            {
                ReturnType result = parse_func();
                if (out_failed) *out_failed = false;
                return result;
            }
            catch (const ParseSyncException&)
            {
                synchronize_with(config);
                if (out_failed) *out_failed = true;
                return fallback_value;
            }
        }

        template <typename ParseFunc>
        void attempt_parse_void(ParseFunc parse_func, const RecoveryConfig& config, bool* out_failed = nullptr)
        {
            try
            {
                parse_func();
                if (out_failed) *out_failed = false;
            }
            catch (const ParseSyncException&)
            {
                synchronize_with(config);
                if (out_failed) *out_failed = true;
            }
        }

        template <typename ReturnType, typename ParseFunc>
        ReturnType try_parse(ParseFunc parse_func, const RecoveryConfig& config, bool* out_failed = nullptr)
        {
            return attempt_parse<ReturnType>(parse_func, config, ReturnType{}, out_failed);
        }

        Token try_consume(TokenType type, ValuascriptErrorCode err, const RecoveryConfig& config,
                          bool* out_failed = nullptr)
        {
            return attempt_parse<Token>(
                [&]() { return cursor.consume(type, err); },
                config,
                Token(type, "<error>", cursor.peek().line, cursor.peek().column),
                out_failed
            );
        }

        Token try_consume_identifier(ValuascriptErrorCode err, const RecoveryConfig& config,
                                     bool allow_top_level = true, bool check_boundary = false,
                                     bool* out_failed = nullptr)
        {
            return attempt_parse<Token>(
                [&]() { return consume_identifier(err, allow_top_level, check_boundary); },
                config,
                Token(TokenType::Identifier, "<error>", cursor.peek().line, cursor.peek().column),
                out_failed
            );
        }

        template <typename ElementType, typename IsElementStart, typename ElementParser>
        std::vector<ElementType> parse_list(
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
                    if (TokenTraits::is_newline_statement_boundary(cursor.previous(), token, next))
                    {
                        if (token.type != TokenType::At || is_at_any_declaration()) return true;
                    }
                    return false;
                }
                if (TokenTraits::is_newline_statement_boundary(cursor.previous(), token, next)) return true;
                for (TokenType stop : recovery_boundaries) if (token.type == stop) return true;
                return false;
            };

            while (!cursor.check(closing_token) && !cursor.is_at_end())
            {
                if (is_at_parent_boundary && is_at_parent_boundary(0)) break;

                try
                {
                    const Token& tok = cursor.peek();
                    TokenType next = cursor.peek(1).type;

                    if ((TokenTraits::is_statement_start(tok, next) ||
                        TokenTraits::is_top_level_only_declaration(tok.type)) && !is_element_start())
                    {
                        if (tok.line > cursor.previous().line) break;
                        else
                        {
                            const Token& start_tok = cursor.peek();
                            if (on_unexpected_statement) on_unexpected_statement();
                            cursor.report_error_no_panic(cursor.make_span(start_tok, cursor.previous()),
                                                         ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
                            throw ParseSyncException();
                        }
                    }

                    elements.push_back(parse_element());
                    if (is_at_parent_boundary && is_at_parent_boundary(0)) break;

                    if (cursor.check(TokenType::Comma))
                    {
                        if (is_at_parent_boundary && is_at_parent_boundary(1)) break;
                        cursor.advance();
                        if (cursor.check(closing_token) && trailing_comma_err)
                            cursor.report_error(
                                cursor.previous(), *trailing_comma_err);
                    }
                    else if (!cursor.check(closing_token))
                    {
                        bool is_boundary = TokenTraits::is_newline_statement_boundary(
                            cursor.previous(), cursor.peek(), cursor.peek(1).type);
                        if (is_boundary && cursor.peek().type == TokenType::At)
                        {
                            if (!is_at_any_declaration()) is_boundary = false;
                        }
                        if (is_element_start() && !is_boundary)
                        {
                            if (missing_comma_err) cursor.report_error_no_panic(cursor.peek(), *missing_comma_err);
                        }
                        else break;
                    }
                }
                catch (const ParseSyncException&)
                {
                    synchronize_with({
                        .stop_tokens = {TokenType::Comma, closing_token},
                        .options = DefaultRecoveryOptions |
                        RecoveryOptions::StopAtBoundaryRespectingDanglingOp |
                        RecoveryOptions::IgnoreStandaloneModifiersAsBoundaries,
                        .custom_stop_predicate = [&](const Token&, TokenType)
                        {
                            if (is_at_parent_boundary && is_at_parent_boundary(0)) return true;
                            return false;
                        }
                    });
                    if (cursor.check(TokenType::Comma))
                    {
                        if (is_at_parent_boundary && is_at_parent_boundary(1)) break;
                        cursor.advance();
                    }
                }
                if (cursor.peek().type != closing_token && is_hard_stop(cursor.peek(), cursor.peek(1).type)) break;
            }
            return elements;
        }

        template <typename ElementType, typename ElementParser>
        std::vector<ElementType> parse_list(
            const TokenType closing_token,
            const std::optional<ValuascriptErrorCode> trailing_comma_err,
            const ValuascriptErrorCode missing_comma_err,
            const std::vector<TokenType>& recovery_boundaries,
            ElementParser parse_element,
            const std::function<bool(int)>& is_at_parent_boundary = nullptr)
        {
            return parse_list<ElementType>(
                closing_token, trailing_comma_err, std::make_optional(missing_comma_err), recovery_boundaries,
                [this]()
                {
                    const Token& tok = cursor.peek();
                    return tok.type == TokenType::Identifier ||
                        TokenTraits::acts_like_identifier(tok, cursor.peek(1).type) || tok.type == TokenType::LeftParen;
                },
                parse_element,
                is_at_parent_boundary
            );
        }

        template <typename T, typename... Args>
        std::unique_ptr<T> make_node(const Token& start_token, Args&&... args)
        {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            node->span = cursor.make_span(start_token, cursor.previous());
            return node;
        }

        template <typename T, typename... Args>
        std::unique_ptr<T> make_node_with_span(const SourceSpan& span, Args&&... args)
        {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            node->span = span;
            return node;
        }
    };
}
