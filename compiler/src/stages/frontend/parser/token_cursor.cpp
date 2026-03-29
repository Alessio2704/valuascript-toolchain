#include "stages/frontend/parser/token_cursor.h"
#include "stages/frontend/parser/parser.h"
#include "errors/error_formatter.h"
#include <algorithm>

namespace valuascript::compiler {
    TokenCursor::TokenCursor(const std::vector<Token> &tokens, std::string file_path,
                             CompilerContext &context)
        : tokens_(tokens), file_path_(std::move(file_path)), context_(context) {
    }

    const Token &TokenCursor::peek(const int num) const {
        return tokens_[current_ + num];
    }

    const Token &TokenCursor::previous(const int num) const {
        return tokens_[current_ - num];
    }

    bool TokenCursor::is_at_end() const {
        return peek().type == TokenType::EndOfFile;
    }

    bool TokenCursor::check(const TokenType type) const {
        if (is_at_end()) return false;
        return peek().type == type;
    }

    const Token &TokenCursor::advance() {
        if (!is_at_end()) current_++;
        return previous();
    }

    bool TokenCursor::match(const std::initializer_list<TokenType> types) {
        if (std::ranges::any_of(types, [this](const TokenType type) { return check(type); })) {
            advance();
            return true;
        }
        return false;
    }

    const Token &TokenCursor::consume(const TokenType type, const ValuascriptErrorCode code) {
        if (check(type)) return advance();
        report_error(peek(), code);
    }

    SourceSpan TokenCursor::make_span(const Token &start_token, const Token &end_token) const {
        size_t end_col = end_token.column;
        if (end_token.type != TokenType::EndOfFile) {
            end_col += end_token.lexeme.length();
        }
        return {start_token.line, start_token.column, end_token.line, end_col, file_path_};
    }

    SourceSpan TokenCursor::combine_spans(const SourceSpan &start, const SourceSpan &end) const {
        return {start.line_start, start.column_start, end.line_end, end.column_end, file_path_};
    }

    void TokenCursor::report_error_no_panic(const SourceSpan &span,
                                            const ValuascriptErrorCode code) const {
        std::string message = format_error(code);

        ValuaScriptException ex(
            ValuascriptErrorCategory::Syntax,
            code,
            {span.line_start, span.column_start, span.line_end, span.column_end, file_path_},
            std::move(message)
        );
        context_.handle_error(ex);
    }

    void TokenCursor::report_error_no_panic(const Token &token,
                                            const ValuascriptErrorCode code,
                                            const bool force_token_location) const {
        size_t err_line = token.line;
        size_t err_column_start = token.column;
        size_t err_column_end = token.column + (token.lexeme.empty() ? 1 : token.lexeme.length());

        if (!force_token_location && current_ > 0) {
            const Token &prev = tokens_[current_ - 1];
            if (token.line > prev.line || token.type == TokenType::EndOfFile) {
                err_line = prev.line;
                err_column_start = prev.column + prev.lexeme.size();
                err_column_end = err_column_start + 1;
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

    [[noreturn]] void TokenCursor::report_error(const Token &token,
                                                const ValuascriptErrorCode code,
                                                const bool force_token_location) const {
        report_error_no_panic(token, code, force_token_location);
        throw ParseSyncException();
    }
}
