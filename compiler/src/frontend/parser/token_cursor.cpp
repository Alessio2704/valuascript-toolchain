#include "token_cursor.h"
#include "parser.h"
#include "token_traits.h"
#include "core/error_formatter.h"
#include <algorithm>

namespace valuascript::compiler
{
    TokenCursor::TokenCursor(const std::vector<Token>& tokens, std::string file_path,
                             CompilerContext& context)
        : tokens_(tokens), file_path_(std::move(file_path)), context_(context)
    {
    }

    const Token& TokenCursor::peek(const int lookahead) const
    {
        size_t target = current_ + lookahead;
        if (target >= tokens_.size())
        {
            return tokens_.back();
        }
        return tokens_[target];
    }

    const Token& TokenCursor::previous(const int lookback) const
    {
        if (current_ < static_cast<size_t>(lookback))
        {
            return tokens_.front();
        }
        return tokens_[current_ - lookback];
    }

    bool TokenCursor::is_at_end() const
    {
        return peek().type == TokenType::EndOfFile;
    }

    bool TokenCursor::check(const TokenType type) const
    {
        if (is_at_end()) return false;
        return peek().type == type;
    }

    const Token& TokenCursor::advance()
    {
        if (!is_at_end()) current_++;
        return previous();
    }

    bool TokenCursor::match(const std::initializer_list<TokenType> types)
    {
        if (std::ranges::any_of(types, [this](const TokenType type) { return check(type); }))
        {
            advance();
            return true;
        }
        return false;
    }

    const Token& TokenCursor::consume(const TokenType type,
                                      const ValuascriptErrorCode code,
                                      bool use_exact_token_range)
    {
        if (check(type)) return advance();
        report_error(peek(), code, use_exact_token_range);
    }

    SourceSpan TokenCursor::make_span(const Token& start_token, const Token& end_token) const
    {
        if (end_token.line < start_token.line ||
            (end_token.line == start_token.line && end_token.column < start_token.column))
        {
            size_t end_col = start_token.column;
            if (start_token.type != TokenType::EndOfFile)
            {
                end_col += start_token.lexeme.length();
            }
            return {start_token.line, start_token.column, start_token.line, end_col, file_path_};
        }

        size_t end_col = end_token.column;
        if (end_token.type != TokenType::EndOfFile)
        {
            end_col += end_token.lexeme.length();
        }
        return {start_token.line, start_token.column, end_token.line, end_col, file_path_};
    }

    SourceSpan TokenCursor::combine_spans(const SourceSpan& start, const SourceSpan& end) const
    {
        return {start.line_start, start.column_start, end.line_end, end.column_end, file_path_};
    }

    void TokenCursor::report_error_no_panic(const SourceSpan& span,
                                            const ValuascriptErrorCode code) const
    {
        if (suppress_errors_) return;
        std::string message = format_error(code);

        ValuaScriptException ex(
            ValuascriptErrorCategory::Syntax,
            code,
            {span.line_start, span.column_start, span.line_end, span.column_end, file_path_},
            std::move(message)
        );
        context_.handle_error(ex);
    }

    void TokenCursor::report_error_no_panic(const Token& token,
                                            const ValuascriptErrorCode code,
                                            const bool use_exact_token_range) const
    {
        if (suppress_errors_) return;
        size_t err_line = token.line;
        size_t err_column_start = token.column;
        size_t err_column_end = token.column + (token.lexeme.empty() ? 1 : token.lexeme.length());

        if (!use_exact_token_range && current_ > 0)
        {
            const Token& prev = tokens_[current_ - 1];

            bool should_shift = token.type == TokenType::EndOfFile;

            if (!should_shift && token.line > prev.line)
            {
                if (TokenTraits::is_newline_statement_boundary(prev, token, peek(1).type))
                {
                    should_shift = true;
                }
                else if (TokenTraits::is_dangling_operator(prev.type) && prev.type != TokenType::Comma)
                {
                    should_shift = true;
                }
                else if (prev.type == TokenType::At || prev.type == TokenType::Hash)
                {
                    should_shift = true;
                }
                else if (code == ValuascriptErrorCode::MissingOperator ||
                    code == ValuascriptErrorCode::MissingCommaOrOperatorBetweenExpressions)
                {
                    should_shift = true;
                }
            }

            if (should_shift)
            {
                size_t final_line = prev.line;
                size_t final_col = prev.column;

                std::string_view view = prev.lexeme;

                size_t last_char = view.find_last_not_of(" \t\n\r");
                if (last_char != std::string_view::npos)
                {
                    view = view.substr(0, last_char + 1);
                }

                for (char c : view)
                {
                    if (c == '\n')
                    {
                        final_line++;
                        final_col = 1;
                    }
                    else
                    {
                        final_col++;
                    }
                }

                err_line = final_line;
                err_column_start = final_col;
                err_column_end = final_col + 1;
            }
        }

        std::string message = format_error(code);

        ValuaScriptException ex(
            ValuascriptErrorCategory::Syntax,
            code,
            {err_line, err_column_start, err_line, err_column_end, file_path_},
            std::move(message)
        );
        context_.handle_error(ex);
    }

    [[noreturn]] void TokenCursor::report_error(const Token& token,
                                                const ValuascriptErrorCode code,
                                                const bool use_exact_token_range) const
    {
        report_error_no_panic(token, code, use_exact_token_range);
        throw ParseSyncException();
    }
}
