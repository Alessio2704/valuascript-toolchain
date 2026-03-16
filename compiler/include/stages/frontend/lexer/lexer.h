#pragma once
#include <string>

#include "token.h"
#include "compiler_context/compiler_context.h"

#include "errors/valuascript_exception.h"
#include "errors/error_formatter.h"

namespace valuascript::compiler {
    class Lexer {
    private:
        std::string source_;
        std::string file_path_;
        std::vector<Token> tokens_;
        std::shared_ptr<CompilerContext> context_;
        size_t start_ = 0;
        size_t current_ = 0;
        size_t line_ = 1;
        size_t line_start_ = 1;
        size_t column_start_ = 1;
        size_t column_current_ = 1;

    public:
        Lexer(std::string source, std::string file_path, std::shared_ptr<CompilerContext> context);

        std::vector<Token> tokenize();

    private:
        [[nodiscard]] bool is_at_end() const { return current_ >= source_.length(); }

        char advance() {
            column_current_++;
            return source_[current_++];
        }

        [[nodiscard]] char peek() const {
            if (is_at_end()) return '\0';
            return source_[current_];
        }

        [[nodiscard]] char peek_next() const {
            if (current_ + 1 >= source_.length()) return '\0';
            return source_[current_ + 1];
        }

        bool match(const char expected) {
            if (is_at_end() || source_[current_] != expected) return false;
            current_++;
            column_current_++;
            return true;
        }

        void add_token(TokenType type) {
            std::string text = source_.substr(start_, current_ - start_);
            tokens_.emplace_back(type, std::move(text), line_, column_start_);
        }

        void scan_string();

        void scan_number();

        void scan_identifier();

        void scan_token();

        template<typename... Args>
        void report_error(const ErrorCode &code, Args &&... args) const {
            std::string message = format_error(code, std::forward<Args>(args)...);

            ValuaScriptException ex(
                ErrorCategory::Lexical,
                code,
                {line_start_, column_start_, line_, column_current_, file_path_},
                std::move(message)
            );

            context_->handle_error(ex);
        }
    };
}
