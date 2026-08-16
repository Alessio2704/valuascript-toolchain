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
            const Token& start = cursor_.consume(open_token_, open_brace_err_);
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
