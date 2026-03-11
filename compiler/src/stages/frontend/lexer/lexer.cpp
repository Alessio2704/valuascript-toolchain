#include "stages/frontend/lexer/lexer.h"

namespace valuascript::compiler {
    Lexer::Lexer(std::string source, std::string file_path, std::shared_ptr<CompilerContext> context)
        : source_(std::move(source)), file_path_(std::move(file_path)), context_(std::move(context)) {
    }

    void Lexer::report_error(const ErrorCode& code, const std::string &message) const {
        ValuaScriptException ex(
            ErrorCategory::Lexical,
            code,
            {line_start_, column_start_, line_, column_current_, file_path_},
            message
        );

        context_->handle_error(ex);
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


    void Lexer::scan_string() {
        bool is_docstring = false;
        if (peek() == '"' && peek_next() == '"') {
            is_docstring = true;
            advance();
            advance();
        }

        while (!is_at_end()) {
            if (is_docstring && peek() == '"' && peek_next() == '"' && source_[current_ + 2] == '"') {
                break;
            }

            if (!is_docstring && peek() == '"') {
                break;
            }

            if (!is_docstring && peek() == '\n') {
                report_error(ErrorCode::UnclosedString, "Syntax Error: Unclosed string literal.");
                return;
            }

            if (peek() == '\n') {
                line_++;
                column_current_ = 1;
            }

            advance();
        }

        if (is_at_end()) {
            report_error(ErrorCode::UnclosedString, "Syntax Error: Unclosed string literal.");
            return;
        }

        if (is_docstring) {
            advance();
            advance();
            advance();
            add_token(TokenType::DocString);
        } else {
            advance();
            add_token(TokenType::String);
        }
    }

    void Lexer::scan_number() {
        auto consume_integer_part = [this]() {
            while (std::isdigit(peek()) || peek() == '_') {
                if (peek() == '_') {
                    if (!std::isdigit(peek_next())) {
                        advance();
                        report_error(
                            ErrorCode::InvalidCharacter,
                            "Syntax Error: Invalid character '_' found."
                        );
                    }
                }
                advance();
            }
        };

        consume_integer_part();

        if (peek() == '.') {
            if (std::isdigit(peek_next())) {
                advance();
                consume_integer_part();
            } else {
                advance();
                report_error(
                    ErrorCode::UnterminatedDecimal,
                    "Syntax Error: Unterminated decimal number. Expected digits after '.'."
                );
            }
        }

        if (peek() == '%') {
            advance();
            add_token(TokenType::PercentageLiteral);
            return;
        }

        add_token(TokenType::Number);
    }

    void Lexer::scan_identifier() {
        while (std::isalnum(peek()) || peek() == '_') advance();

        const std::string text = source_.substr(start_, current_ - start_);

        if (auto keyword_opt = get_keyword_type(text); keyword_opt.has_value()) {
            add_token(keyword_opt.value());
        } else {
            add_token(TokenType::Identifier);
        }
    }

    void Lexer::scan_token() {
        switch (const char c = advance()) {
            case '(': add_token(TokenType::LeftParen);
                break;
            case ')': add_token(TokenType::RightParen);
                break;
            case '[': add_token(TokenType::LeftBracket);
                break;
            case ']': add_token(TokenType::RightBracket);
                break;
            case '{': add_token(TokenType::LeftBrace);
                break;
            case '}': add_token(TokenType::RightBrace);
                break;
            case ',': add_token(TokenType::Comma);
                break;
            case ':': add_token(TokenType::Colon);
                break;
            case '+': add_token(TokenType::Plus);
                break;
            case '-': add_token(match('>') ? TokenType::Arrow : TokenType::Minus);
                break;
            case '*': add_token(TokenType::Star);
                break;
            case '^': add_token(TokenType::Caret);
                break;
            case '.': {
                if (std::isdigit(peek())) {
                    bool is_member_access = false;

                    if (!tokens_.empty()) {
                        TokenType last_type = tokens_.back().type;
                        if (last_type == TokenType::Identifier ||
                            last_type == TokenType::RightParen ||
                            last_type == TokenType::RightBracket) {
                            is_member_access = true;
                        }
                    }

                    if (is_member_access) {
                        add_token(TokenType::Dot);
                    } else {
                        report_error(
                            ErrorCode::DecimalMissingLeadingZero,
                            "Syntax Error: Decimals must start with a leading zero (e.g., '0.5' instead of '.5')."
                        );
                    }
                } else {
                    add_token(TokenType::Dot);
                }
                break;
            }

            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !is_at_end()) advance();
                } else {
                    add_token(TokenType::Slash);
                }
                break;

            case '#': add_token(TokenType::Hash);
                break;

            case '@': add_token(TokenType::At);
                break;
            case '=': add_token(match('=') ? TokenType::Equals : TokenType::Assign);
                break;
            case '!': if (match('=')) add_token(TokenType::NotEquals);
                break;
            case '<': add_token(match('=') ? TokenType::LessEqual : TokenType::Less);
                break;
            case '>': add_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
                break;

            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                line_++;
                column_current_ = 1;
                break;

            case '"': scan_string();
                break;

            default:
                if (std::isdigit(c)) {
                    scan_number();
                } else if (std::isalpha(c) || c == '_') {
                    scan_identifier();
                } else {
                    std::string msg = "Syntax Error: Invalid character '";
                    msg += c;
                    msg += "' found.";

                    report_error(
                        ErrorCode::InvalidCharacter,
                        msg
                    );
                }
                break;
        }
    }
}
