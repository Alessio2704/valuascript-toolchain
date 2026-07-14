#pragma once

#include <vector>
#include <string>
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
        std::string file_path_;
        size_t current_ = 0;
        CompilerContext& context_;
        bool suppress_errors_ = false;

    public:
        TokenCursor(const std::vector<Token>& tokens, std::string file_path, CompilerContext& context);

        void set_suppress_errors(bool suppress) { suppress_errors_ = suppress; }
        [[nodiscard]] bool get_suppress_errors() const { return suppress_errors_; }

        [[nodiscard]] const Token& peek(size_t lookahead = 0) const;

        [[nodiscard]] const Token& previous(size_t lookback = 1) const;

        [[nodiscard]] bool is_at_end() const;

        [[nodiscard]] bool check(TokenType type) const;

        const Token& advance();

        bool match(std::initializer_list<TokenType> types);

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
