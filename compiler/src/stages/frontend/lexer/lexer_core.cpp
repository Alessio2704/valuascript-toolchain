#include "stages/frontend/lexer/lexer.h"

namespace valuascript::compiler {
    Lexer::Lexer(std::string source, std::string file_path, CompilerContext &context)
        : source_(std::move(source)), file_path_(std::move(file_path)), context_(context) {
    }

    std::vector<Token> Lexer::tokenize() {
        while (!is_at_end()) {
            start_ = current_;
            column_start_ = column_current_;
            line_start_ = line_;
            scan_token();
        }
        tokens_.emplace_back(TokenType::EndOfFile, "", line_, column_current_);
        return tokens_;
    }

    [[nodiscard]] bool Lexer::is_at_end() const { return current_ >= source_.length(); }

    char Lexer::advance() {
        column_current_++;
        return source_[current_++];
    }

    [[nodiscard]] char Lexer::peek() const {
        if (is_at_end()) return '\0';
        return source_[current_];
    }

    [[nodiscard]] char Lexer::peek_next() const {
        if (current_ + 1 >= source_.length()) return '\0';
        return source_[current_ + 1];
    }

    bool Lexer::match(const char expected) {
        if (is_at_end() || source_[current_] != expected) return false;
        current_++;
        column_current_++;
        return true;
    }

    void Lexer::add_token(TokenType type) {
        std::string text = source_.substr(start_, current_ - start_);
        tokens_.emplace_back(type, std::move(text), line_, column_start_);
    }
}
