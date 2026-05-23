#pragma once
#include "parser_context.h"
#include <vector>
#include <functional>
#include <optional>

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
        std::function<bool(const Token & tok, TokenType next)> custom_stop_predicate = nullptr;

        [[nodiscard]] bool has(RecoveryOptions opt) const { return (options & opt) != RecoveryOptions::None; }

        static RecoveryConfig StopAtBoundary(std::vector<TokenType> extra_stops = {})
        {
            return {
                std::move(extra_stops), DefaultRecoveryOptions | RecoveryOptions::StopAtBoundaryRespectingDanglingOp,
                nullptr
            };
        }

        static RecoveryConfig ForceStopAtBoundary(std::vector<TokenType> extra_stops = {})
        {
            return {
                std::move(extra_stops), DefaultRecoveryOptions | RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp,
                nullptr
            };
        }

        static RecoveryConfig StopAtNewline()
        {
            return {{}, DefaultRecoveryOptions | RecoveryOptions::StopAtNewline, nullptr};
        }
    };

    struct CloserTracker
    {
        ParserContext& ctx;
        CloserTracker(ParserContext& c, TokenType t);
        ~CloserTracker();
    };

    struct SyncSetTracker
    {
        ParserContext& ctx;
        size_t previous_size;
        SyncSetTracker(ParserContext& c, const std::vector<TokenType>& tokens);
        ~SyncSetTracker();
    };

    class ErrorRecovery
    {
    public:
        using SyncPredicate = std::function<bool(TokenType type, int nesting_depth)>;
        static void recover(ParserContext& ctx, const SyncPredicate& stop_condition);
        static void synchronize_with(ParserContext& ctx, const RecoveryConfig& config);
        static void synchronize_to_closer(ParserContext& ctx, TokenType closing_token);
        static void synchronize_and_consume_closer(ParserContext& ctx, TokenType expected_closer);

        template <typename ReturnType, typename ParseFunc>
        static ReturnType attempt_parse(ParserContext& ctx, ParseFunc parse_func, const RecoveryConfig& config,
                                        ReturnType fallback_value, bool* out_failed = nullptr)
        {
            try
            {
                ReturnType result = parse_func();
                if (out_failed) *out_failed = false;
                return result;
            }
            catch (const ParseSyncException&)
            {
                synchronize_with(ctx, config);
                if (out_failed) *out_failed = true;
                return fallback_value;
            }
        }

        template <typename ParseFunc>
        static void attempt_parse_void(ParserContext& ctx, ParseFunc parse_func, const RecoveryConfig& config,
                                       bool* out_failed = nullptr)
        {
            try
            {
                parse_func();
                if (out_failed) *out_failed = false;
            }
            catch (const ParseSyncException&)
            {
                synchronize_with(ctx, config);
                if (out_failed) *out_failed = true;
            }
        }

        template <typename ReturnType, typename ParseFunc>
        static ReturnType try_parse(ParserContext& ctx, ParseFunc parse_func, const RecoveryConfig& config,
                                    bool* out_failed = nullptr)
        {
            return attempt_parse<ReturnType>(ctx, parse_func, config, ReturnType{}, out_failed);
        }

        static Token try_consume(ParserContext& ctx, TokenType type, ParserErrorCode err,
                                 const RecoveryConfig& config, bool* out_failed = nullptr)
        {
            return attempt_parse<Token>(
                ctx, [&]() { return ctx.cursor.consume(type, err); }, config,
                Token(type, "<error>", ctx.cursor.peek().line, ctx.cursor.peek().column), out_failed
            );
        }

        static Token try_consume_identifier(ParserContext& ctx, ParserErrorCode err, const RecoveryConfig& config,
                                            bool allow_top_level = true, bool check_boundary = false,
                                            bool* out_failed = nullptr)
        {
            return attempt_parse<Token>(
                ctx, [&]() { return ctx.consume_identifier(err, allow_top_level, check_boundary); }, config,
                Token(TokenType::Identifier, "<error>", ctx.cursor.peek().line, ctx.cursor.peek().column), out_failed
            );
        }
    };
}
