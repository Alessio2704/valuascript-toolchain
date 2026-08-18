#pragma once
#include <optional>
#include <functional>
#include <vector>
#include <string>
#include "parser_context.h"
#include "error_recovery.h"
#include "parser.h"

namespace valuascript::compiler
{
    class Parser;

    class BlockParser
    {
    private:
        Parser& parser_;
        ParserContext& ctx_;
        TokenCursor& cursor_;

        TokenType open_token_ = TokenType::LeftBrace;
        TokenType close_token_ = TokenType::RightBrace;

        ParserErrorCode open_brace_err_ = ParserErrorCode::ExpectedLeftBraceBeforeFunctionBody;
        ParserErrorCode close_brace_err_ = ParserErrorCode::ExpectedRightBraceAfterFunctionBody;

        RecoveryConfig body_recovery_config_{
            .options = RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp
        };

        std::function<void()> on_enter_block_ = nullptr;
        std::function<bool()> is_at_terminating_declaration_ = nullptr;

    public:
        explicit BlockParser(Parser& parser)
            : parser_(parser), ctx_(parser.ctx), cursor_(parser.ctx.cursor)
        {
        }

        BlockParser& on_missing_open_brace(ParserErrorCode err)
        {
            open_brace_err_ = err;
            return *this;
        }

        BlockParser& on_missing_close_brace(ParserErrorCode err)
        {
            close_brace_err_ = err;
            return *this;
        }

        BlockParser& with_recovery_config(RecoveryConfig config)
        {
            body_recovery_config_ = std::move(config);
            return *this;
        }

        BlockParser& terminates_on_declaration(std::function<bool()> predicate)
        {
            is_at_terminating_declaration_ = std::move(predicate);
            return *this;
        }

        BlockParser& on_enter_block(std::function<void()> callback)
        {
            on_enter_block_ = std::move(callback);
            return *this;
        }

        BlockParser& collect_docstring(std::optional<std::string>& out_docstring)
        {
            on_enter_block_ = [this, &out_docstring]()
            {
                if (cursor_.check(TokenType::DocString))
                {
                    out_docstring = cursor_.advance().lexeme;
                }
            };
            return *this;
        }

        template <typename ItemParser>
        SourceSpan parse_body(ItemParser parse_item)
        {
            Token start = cursor_.peek();
            bool has_open_brace = cursor_.check(open_token_);
            if (has_open_brace)
            {
                start = cursor_.advance();
            }
            else
            {
                cursor_.report_error_no_panic(cursor_.peek(), open_brace_err_);
            }

            auto active_closers_count = std::count(ctx_.active_closers.begin(), ctx_.active_closers.end(), close_token_);
            int64_t net_closers_ahead = 0;
            size_t offset = 0;
            while (true)
            {
                TokenType t = cursor_.peek(offset).type;
                if (t == TokenType::EndOfFile) break;
                if (t == open_token_) net_closers_ahead--;
                else if (t == close_token_) net_closers_ahead++;
                offset++;
            }
            bool has_matching_closer_ahead = net_closers_ahead > active_closers_count;

            if (!has_open_brace && !has_matching_closer_ahead)
            {
                return cursor_.make_span(start, cursor_.previous());
            }

            CloserTracker tracker(ctx_, close_token_, ContainerKind::Block);

            if (on_enter_block_)
            {
                on_enter_block_();
            }

            auto should_break_early = [&]()
            {
                if (!ctx_.is_missing_closing_brace()) return false;
                if (is_at_terminating_declaration_) return is_at_terminating_declaration_();
                return ctx_.is_at_top_level_declaration();
            };

            while (!cursor_.check(close_token_) && !cursor_.is_at_end())
            {
                if (should_break_early())
                {
                    break;
                }
                ErrorRecovery::attempt_parse_void(ctx_, parse_item, body_recovery_config_);
            }

            Token end_token = cursor_.previous();
            try
            {
                end_token = cursor_.consume(close_token_, close_brace_err_);
            }
            catch (const ParseSyncException&)
            {
                ErrorRecovery::synchronize_and_consume_closer(ctx_, close_token_);
                end_token = cursor_.previous();
            }

            return cursor_.make_span(start, end_token);
        }

        SourceSpan parse_statements(ParseContextType context, std::vector<StmtPtr>& out_body)
        {
            return parse_body([&]()
            {
                parser_.parse_statement_or_declaration(context, nullptr, nullptr, out_body);
            });
        }
    };
}
