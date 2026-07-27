#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <initializer_list>
#include <stdexcept>

#include "core/compiler_context.h"
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
        const std::vector<Token>& tokens_;
        std::shared_ptr<const std::string> file_path_;
        size_t current_ = 0;
        CompilerContext& context_;
        bool suppress_errors_ = false;

    public:
        TokenCursor(const std::vector<Token>& tokens, std::shared_ptr<const std::string> file_path, CompilerContext& context);
        TokenCursor(const std::vector<Token>& tokens, std::string file_path, CompilerContext& context);

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
            if (((tok.type == types) || ...))
            {
                advance();
                return true;
            }
            return false;
        }

        const Token& consume(TokenType type, ParserErrorCode code, bool use_exact_token_range = false);

        [[nodiscard]] SourceSpan make_span(const Token& start_token, const Token& end_token) const;

        [[nodiscard]] SourceSpan combine_spans(const SourceSpan& start, const SourceSpan& end) const;

        void report_error_no_panic(const SourceSpan& span, ParserErrorCode code) const;

        void report_error_no_panic(const Token& token, ParserErrorCode code,
                                   bool use_exact_token_range = false) const;

        [[noreturn]] void report_error(const SourceSpan& span, ParserErrorCode code) const;

        [[noreturn]] void report_error(const Token& token, ParserErrorCode code,
                                       bool use_exact_token_range = false) const;
    };
}
