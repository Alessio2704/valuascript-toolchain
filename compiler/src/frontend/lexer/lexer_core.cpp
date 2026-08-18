#include "lexer.h"

namespace valuascript::compiler
{
    Lexer::Lexer(std::string_view source, std::string file_path, CompilerContext& context)
        : file_path_(std::move(file_path)), context_(context)
    {
        source_ = context_.source_manager.register_source(file_path_, std::string(source));
    }

    std::vector<Token> Lexer::tokenize()
    {
        while (!is_at_end())
        {
            start_ = current_;
            column_start_ = column_current_;
            line_start_ = line_;
            scan_token();
        }
        tokens_.push_back(Token{.type = TokenType::EndOfFile, .lexeme = "", .line = line_, .column = column_current_});
        return tokens_;
    }

    [[nodiscard]] bool Lexer::is_at_end() const { return current_ >= source_.length(); }

    char Lexer::advance()
    {
        if (is_at_end()) return '\0';
        column_current_++;
        return source_[current_++];
    }

    [[nodiscard]] char Lexer::peek() const
    {
        if (is_at_end()) return '\0';
        return source_[current_];
    }

    [[nodiscard]] char Lexer::peek_next() const
    {
        if (current_ + 1 >= source_.length()) return '\0';
        return source_[current_ + 1];
    }

    bool Lexer::match(const char expected)
    {
        if (is_at_end() || source_[current_] != expected) return false;
        current_++;
        column_current_++;
        return true;
    }

    void Lexer::add_token(TokenType type)
    {
        size_t length = (current_ > start_) ? (current_ - start_) : 0;
        std::string_view text = source_.substr(start_, length);
        tokens_.push_back(Token{.type = type, .lexeme = text, .line = line_start_, .column = column_start_});
    }

    void Lexer::add_token(TokenType type, std::string_view text)
    {
        tokens_.push_back(Token{.type = type, .lexeme = text, .line = line_, .column = column_start_});
    }
}
