#pragma once

#include <span>
#include <vector>
#include <string>
#include <string_view>
#include <initializer_list>
#include <stdexcept>

#include "core/compiler_context.h"
#include "core/error_formatter.h"
#include "token/token.h"
#include "ast.h"

using namespace valuascript::shared;

namespace valuascript::compiler
{
    struct ParseSyncException : std::exception
    {
        [[nodiscard]] const char* what() const noexcept override
        {
            return "Parser panic mode triggered.";
        }
    };

    class TokenCursor
    {
    private:
        std::span<const Token> tokens_;
        std::shared_ptr<const std::string> file_path_;
        size_t current_ = 0;
        CompilerContext& context_;
        bool suppress_errors_ = false;

    public:
        TokenCursor(std::span<const Token> tokens, std::shared_ptr<const std::string> file_path, CompilerContext& context);
        TokenCursor(std::span<const Token> tokens, std::string file_path, CompilerContext& context);

        [[nodiscard]] std::span<const Token> tokens() const noexcept { return tokens_; }
        [[nodiscard]] size_t current() const noexcept { return current_; }
        [[nodiscard]] size_t size() const noexcept { return tokens_.size(); }
        [[nodiscard]] bool empty() const noexcept { return tokens_.empty(); }
        [[nodiscard]] std::span<const Token> remaining() const noexcept { return tokens_.subspan(current_); }
        [[nodiscard]] CompilerContext& context() noexcept { return context_; }
        [[nodiscard]] const CompilerContext& context() const noexcept { return context_; }
        [[nodiscard]] const std::vector<CommentToken>& get_comments() const
        {
            return context_.get_comments(file_path_ ? *file_path_ : "");
        }

        void set_suppress_errors(bool suppress) { suppress_errors_ = suppress; }
        [[nodiscard]] bool get_suppress_errors() const { return suppress_errors_; }

        [[nodiscard]] inline const Token& peek(const size_t lookahead = 0) const
        {
            const size_t target = current_ + lookahead;
            if (target < tokens_.size()) [[likely]]
            {
                return tokens_[target];
            }
            return tokens_.back();
        }

        [[nodiscard]] inline const Token& previous(const size_t lookback = 1) const
        {
            if (current_ >= lookback) [[likely]]
            {
                return tokens_[current_ - lookback];
            }
            return tokens_.front();
        }

        [[nodiscard]] inline bool is_at_end() const
        {
            return peek().type == TokenType::EndOfFile;
        }

        [[nodiscard]] inline bool check(const TokenType type) const
        {
            const Token& tok = peek();
            return tok.type == type && tok.type != TokenType::EndOfFile;
        }

        inline const Token& advance()
        {
            if (current_ < tokens_.size() && tokens_[current_].type != TokenType::EndOfFile) [[likely]]
            {
                return tokens_[current_++];
            }
            return tokens_.back();
        }

        template <typename... Ts>
        inline bool match(Ts... types)
        {
            const Token& tok = peek();
            if (tok.type == TokenType::EndOfFile) [[unlikely]] return false;
            if (((tok.type == types) || ...)) [[likely]]
            {
                advance();
                return true;
            }
            return false;
        }

        const Token& consume(TokenType type, ParserErrorCode code, bool use_exact_token_range = false);

        [[nodiscard]] SourceSpan make_span(const Token& start_token, const Token& end_token) const;

        [[nodiscard]] SourceSpan combine_spans(const SourceSpan& start, const SourceSpan& end) const;

        [[nodiscard]] SourceSpan compute_token_span(const Token& token, bool use_exact_token_range, ParserErrorCode code) const;

        template <typename... Args>
        void report_error_no_panic(const SourceSpan& span, ParserErrorCode code, Args&&... args) const
        {
            if (suppress_errors_) return;
            std::string message = format_error(code, std::forward<Args>(args)...);

            ValuaScriptException ex(
                ValuascriptErrorCategory::Syntax,
                code,
                SourceSpan{
                    .line_start = span.line_start,
                    .column_start = span.column_start,
                    .line_end = span.line_end,
                    .column_end = span.column_end,
                    .file_path = file_path_
                },
                std::move(message)
            );
            context_.handle_error(ex);
        }

        template <typename... Args>
        void report_error_no_panic(const Token& token, ParserErrorCode code, bool use_exact_token_range, Args&&... args) const
        {
            if (suppress_errors_) return;
            SourceSpan span = compute_token_span(token, use_exact_token_range, code);
            std::string message = format_error(code, std::forward<Args>(args)...);

            ValuaScriptException ex(
                ValuascriptErrorCategory::Syntax,
                code,
                std::move(span),
                std::move(message)
            );
            context_.handle_error(ex);
        }

        template <typename... Args>
        void report_error_no_panic(const Token& token, ParserErrorCode code, Args&&... args) const
        {
            report_error_no_panic(token, code, false, std::forward<Args>(args)...);
        }

        template <typename... Args>
        [[noreturn]] void report_error(const SourceSpan& span, ParserErrorCode code, Args&&... args) const
        {
            report_error_no_panic(span, code, std::forward<Args>(args)...);
            throw ParseSyncException();
        }

        template <typename... Args>
        [[noreturn]] void report_error(const Token& token, ParserErrorCode code, bool use_exact_token_range, Args&&... args) const
        {
            report_error_no_panic(token, code, use_exact_token_range, std::forward<Args>(args)...);
            throw ParseSyncException();
        }

        template <typename... Args>
        [[noreturn]] void report_error(const Token& token, ParserErrorCode code, Args&&... args) const
        {
            report_error_no_panic(token, code, false, std::forward<Args>(args)...);
            throw ParseSyncException();
        }
    };
}
