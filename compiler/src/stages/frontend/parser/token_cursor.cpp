#include "stages/frontend/parser/token_cursor.h"

namespace valuascript::compiler {
    TokenCursor::TokenCursor(const std::vector<Token> &tokens, std::string file_path)
        : tokens_(tokens), file_path_(std::move(file_path)) {
    }

    const Token &TokenCursor::peek() const {
        return tokens_[current_];
    }

    const Token &TokenCursor::previous() const {
        return tokens_[current_ - 1];
    }

    bool TokenCursor::is_at_end() const {
        return peek().type == TokenType::EndOfFile;
    }

    bool TokenCursor::check(TokenType type) const {
        if (is_at_end()) return false;
        return peek().type == type;
    }

    const Token &TokenCursor::advance() {
        if (!is_at_end()) current_++;
        return previous();
    }

    bool TokenCursor::match(std::initializer_list<TokenType> types) {
        for (const TokenType type: types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    const Token &TokenCursor::consume(TokenType type, ErrorCode code, const std::string &message) {
        if (check(type)) return advance();
        throw error(peek(), code, message);
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

    ValuaScriptException TokenCursor::error(const Token &token, ErrorCode code, const std::string &message) const {
        return ValuaScriptException(
            ErrorCategory::Syntax,
            code,
            {token.line, token.column, file_path_},
            message
        );
    }
}
