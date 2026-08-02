#pragma once
#include <string>

#include "core/compiler_context.h"

#include "core/valuascript_exception.h"
#include "core/error_formatter.h"
#include "lexer_error_code.h"
#include "token/token.h"

namespace valuascript::compiler
{
    class Lexer
    {
    private:
        std::string_view source_;
        std::string file_path_;
        std::vector<Token> tokens_;
        CompilerContext& context_;
        size_t start_ = 0;
        size_t current_ = 0;
        size_t line_ = 1;
        size_t line_start_ = 1;
        size_t column_start_ = 1;
        size_t column_current_ = 1;

    public:
        Lexer(std::string_view source, std::string file_path, CompilerContext& context);

        std::vector<Token> tokenize();

    private:
        [[nodiscard]] bool is_at_end() const;
        char advance();
        [[nodiscard]] char peek() const;
        [[nodiscard]] char peek_next() const;
        bool match(char expected);

        void add_token(TokenType type);
        void add_token(TokenType type, std::string_view text);

        void scan_string();

        void consume_digits();

        void finalize_number();

        void scan_number();

        void scan_identifier();

        void scan_token();

        template <typename... Args>
        void report_error(const LexerErrorCode& code, Args&&... args) const
        {
            std::string message = format_error(code, std::forward<Args>(args)...);

            ValuaScriptException ex(
                ValuascriptErrorCategory::Lexical,
                code,
                SourceSpan{
                    .line_start = line_start_,
                    .column_start = column_start_,
                    .line_end = line_,
                    .column_end = column_current_,
                    .file_path = std::make_shared<const std::string>(file_path_)
                },
                std::move(message)
            );

            context_.handle_error(ex);
        }
    };
}
