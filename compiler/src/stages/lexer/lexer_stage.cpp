#include "stages/lexer/lexer_stage.h"
#include "stages/lexer/token.h"
#include "errors/valuascript_exception.h"
#include <unordered_map>

namespace valuascript::compiler {
    namespace {
        const std::unordered_map<std::string, TokenType> kKeywords = {
            {"let", TokenType::Let},
            {"if", TokenType::If},
            {"then", TokenType::Then},
            {"else", TokenType::Else},
            {"true", TokenType::True},
            {"false", TokenType::False},
            {"and", TokenType::And},
            {"or", TokenType::Or},
            {"not", TokenType::Not},
            {"func", TokenType::Func},
            {"return", TokenType::Return},
            {"scalar", TokenType::TypeScalar},
            {"vector", TokenType::TypeVector},
            {"bool", TokenType::TypeBool},
            {"string", TokenType::TypeString}
        };

        class Lexer {
        private:
            std::string source_;
            std::string file_path_;
            std::vector<Token> tokens_;

            size_t start_ = 0;
            size_t current_ = 0;
            size_t line_ = 1;
            size_t column_start_ = 1;
            size_t column_current_ = 1;

        public:
            Lexer(std::string source, std::string file_path)
                : source_(std::move(source)), file_path_(std::move(file_path)) {
            }

            std::vector<Token> tokenize() {
                while (!is_at_end()) {
                    start_ = current_;
                    column_start_ = column_current_;
                    scan_token();
                }
                tokens_.emplace_back(TokenType::EndOfFile, "", line_, column_current_);
                return tokens_;
            }

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

            void scan_string() {
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

                    if (peek() == '\n') {
                        line_++;
                        column_current_ = 1;
                    }

                    advance();
                }

                if (is_at_end()) {
                    throw ValuaScriptException(
                        ErrorCategory::Lexical,
                        ErrorCode::UnclosedString,
                        {line_, column_start_, file_path_},
                        "Syntax Error: Unclosed string literal."
                    );
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

            void scan_number() {
                auto consume_integer_part = [this]() {
                    while (std::isdigit(peek()) || peek() == '_') {
                        if (peek() == '_') {
                            if (!std::isdigit(peek_next())) {
                                advance();
                                throw ValuaScriptException(
                                    ErrorCategory::Lexical,
                                    ErrorCode::InvalidCharacter,
                                    {line_, column_start_, file_path_},
                                    "Syntax Error: Invalid character '_' found."
                                );
                            }
                        }
                        advance();
                    }
                };

                consume_integer_part();

                if (peek() == '.' && std::isdigit(peek_next())) {
                    advance();
                    consume_integer_part();
                }

                add_token(TokenType::Number);
            }

            void scan_identifier() {
                while (std::isalnum(peek()) || peek() == '_') advance();

                const std::string text = source_.substr(start_, current_ - start_);

                if (const auto it = kKeywords.find(text); it != kKeywords.end()) {
                    add_token(it->second);
                } else {
                    add_token(TokenType::Identifier);
                }
            }

            void scan_token() {
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

                    case '/':
                        if (match('/')) {
                            while (peek() != '\n' && !is_at_end()) advance();
                        } else {
                            add_token(TokenType::Slash);
                        }
                        break;

                    case '#':
                        while (peek() != '\n' && !is_at_end()) advance();
                        break;

                    case '@':
                        if (source_.substr(current_, 6) == "import") {
                            for (int i = 0; i < 6; i++) advance();
                            add_token(TokenType::Import);
                        } else {
                            add_token(TokenType::At);
                        }
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

                            throw ValuaScriptException(
                                ErrorCategory::Lexical,
                                ErrorCode::InvalidCharacter,
                                {line_, column_start_, file_path_},
                                msg
                            );
                        }
                        break;
                }
            }
        };
    }

    LexerStage::LexerStage()
        : CompilerStage(
            "LexerStage",
            CompilerStageArtifactCode::TokenStream,
            {CompilerStageArtifactCode::SourceCode, CompilerStageArtifactCode::FilePath}
        ) {
    }

    CompilerStageArtifact LexerStage::run(const std::vector<CompilerStageArtifact> &artifacts) {
        const auto source = extract_artifact_data<std::string>(artifacts, CompilerStageArtifactCode::SourceCode);
        const auto file_path = extract_artifact_data<std::string>(artifacts, CompilerStageArtifactCode::FilePath);

        Lexer lexer(source, file_path);
        std::vector<Token> tokens = lexer.tokenize();

        return {CompilerStageArtifactCode::TokenStream, tokens};
    }
}
