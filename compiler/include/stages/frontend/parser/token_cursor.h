#pragma once

#include <vector>
#include <string>
#include <initializer_list>
#include "stages/frontend/lexer/token.h"
#include "errors/valuascript_exception.h"
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler {
    class TokenCursor {
    private:
        const std::vector<Token> &tokens_;
        std::string file_path_;
        size_t current_ = 0;

    public:
        TokenCursor(const std::vector<Token> &tokens, std::string file_path);

        [[nodiscard]] const Token &peek() const;

        [[nodiscard]] const Token &previous() const;

        [[nodiscard]] bool is_at_end() const;

        [[nodiscard]] bool check(TokenType type) const;

        const Token &advance();

        bool match(std::initializer_list<TokenType> types);

        const Token &consume(TokenType type, ErrorCode code, const std::string &message);

        [[nodiscard]] SourceSpan make_span(const Token &start_token, const Token &end_token) const;

        [[nodiscard]] SourceSpan combine_spans(const SourceSpan &start, const SourceSpan &end) const;

        [[nodiscard]] ValuaScriptException error(const Token &token, ErrorCode code, const std::string &message) const;
    };
}
