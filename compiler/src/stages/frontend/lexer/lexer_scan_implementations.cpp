#include "stages/frontend/lexer/lexer.h"
#include "errors/error_formatter.h"
#include "token/reserved_keyword_lookup.h"

using namespace valuascript::shared;

namespace valuascript::compiler {
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
                report_error(ValuascriptErrorCode::UnclosedString);
                return;
            }

            if (peek() == '\n') {
                line_++;
                column_current_ = 1;
            }

            advance();
        }

        if (is_at_end()) {
            report_error(ValuascriptErrorCode::UnclosedString);
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
                        report_error(ValuascriptErrorCode::TrailingSeparatorInNumberLiteral, "_");
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
                report_error(ValuascriptErrorCode::UnterminatedDecimal);
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

        std::string text = source_.substr(start_, current_ - start_);

        if (const auto keyword_opt = get_keyword_type(text); keyword_opt.has_value()) {
            add_token(keyword_opt.value(), std::move(text));
        } else {
            add_token(TokenType::Identifier, std::move(text));
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
                            last_type == TokenType::RightBracket ||
                            last_type == TokenType::RightBrace) {
                            is_member_access = true;
                        }
                    }

                    if (is_member_access) {
                        add_token(TokenType::Dot);
                    } else {
                        report_error(ValuascriptErrorCode::DecimalMissingLeadingZero);
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
                    report_error(ValuascriptErrorCode::InvalidCharacter, c);
                }
                break;
        }
    }
}
