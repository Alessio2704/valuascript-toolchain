#include "lexer.h"
#include "core/error_formatter.h"
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
            if (is_docstring) {
                if (peek() == '"' && peek_next() == '"') {
                    if (current_ + 2 < source_.length() && source_[current_ + 2] == '"') {
                        advance();
                        advance();
                        advance();
                        add_token(TokenType::DocString);
                        return;
                    }
                }
            } else {
                if (peek() == '"') {
                    advance();
                    add_token(TokenType::String);
                    return;
                }

                if (peek() == '\n' || peek() == '\r') {
                    report_error(ValuascriptErrorCode::UnclosedString);
                    add_token(TokenType::String);
                    return;
                }
            }

            if (peek() == '\n') {
                line_++;
                column_current_ = 1;
            }

            advance();
        }

        report_error(ValuascriptErrorCode::UnclosedString);
        add_token(is_docstring ? TokenType::DocString : TokenType::String);
    }

    void Lexer::consume_digits() {
        while (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '_') {
            if (peek() == '_') {
                if (!std::isdigit(static_cast<unsigned char>(peek_next()))) {
                    advance();
                    report_error(ValuascriptErrorCode::TrailingSeparatorInNumberLiteral);
                    break;
                }
            }
            advance();
        }
    }

    void Lexer::finalize_number() {
        if (match('%')) {
            add_token(TokenType::PercentageLiteral);
        } else {
            add_token(TokenType::Number);
        }
    }

    bool Lexer::is_member_access() const {
        if (tokens_.empty()) return false;
        TokenType last = tokens_.back().type;
        return (last == TokenType::Identifier ||
                last == TokenType::RightParen ||
                last == TokenType::RightBracket ||
                last == TokenType::RightBrace);
    }

    void Lexer::scan_number() {
        consume_digits();

        if (peek() == '.') {
            if (std::isdigit(static_cast<unsigned char>(peek_next()))) {
                advance();
                consume_digits();
            } else {
                advance();
                report_error(ValuascriptErrorCode::UnterminatedDecimal);
            }
        }
        finalize_number();
    }

    void Lexer::scan_identifier() {
        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') advance();

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
            case '*': add_token(TokenType::Star);
                break;
            case '^': add_token(TokenType::Caret);
                break;
            case '#': add_token(TokenType::Hash);
                break;
            case '@': add_token(TokenType::At);
                break;
            case '-': add_token(match('>') ? TokenType::Arrow : TokenType::Minus);
                break;
            case '=': add_token(match('=') ? TokenType::Equals : TokenType::Assign);
                break;
            case '<': add_token(match('=') ? TokenType::LessEqual : TokenType::Less);
                break;
            case '>': add_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
                break;
            case '!':
                if (match('=')) {
                    add_token(TokenType::NotEquals);
                } else {
                    report_error(ValuascriptErrorCode::InvalidCharacter, c);
                }
                break;
            case '.':
                if (std::isdigit(static_cast<unsigned char>(peek()))) {
                    if (is_member_access()) {
                        add_token(TokenType::Dot);
                    } else {
                        report_error(ValuascriptErrorCode::DecimalMissingLeadingZero);
                        consume_digits();
                        finalize_number();
                    }
                } else {
                    add_token(TokenType::Dot);
                }
                break;
            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !is_at_end()) advance();
                } else {
                    add_token(TokenType::Slash);
                }
                break;
            case '\r':
            case ' ':
            case '\t':
                break;
            case '\n':
                line_++;
                column_current_ = 1;
                break;
            case '"':
                scan_string();
                break;
            default:
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    scan_number();
                } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                    scan_identifier();
                } else {
                    report_error(ValuascriptErrorCode::InvalidCharacter, c);
                }
                break;
        }
    }
}
