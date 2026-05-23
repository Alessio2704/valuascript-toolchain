#pragma once
#include <frontend/parser/ast_core.h>
#include <string>
#include <utility>
#include <variant>
#include <ostream>
#include "frontend/file_reader/file_reader_error_code.h"
#include "frontend/lexer/lexer_error_code.h"
#include "frontend/parser/parser_error_code.h"
#include "midend/project_resolver/project_resolver_error_code.h"

namespace valuascript::compiler
{
    enum class ValuascriptErrorCategory { File, Lexical, Syntax, Semantic, Import, Internal };

    using ValuascriptErrorCode = std::variant<
        FileReaderErrorCode,
        LexerErrorCode,
        ParserErrorCode,
        ProjectResolverErrorCode
    >;

    inline std::ostream& operator<<(std::ostream& os, const ValuascriptErrorCode& code)
    {
        int num = std::visit([](auto&& c) { return static_cast<int>(c); }, code);
        std::string_view tmpl = std::visit([](auto&& c) { return get_error_template(c); }, code);
        return os << "E" << num << " (\"" << tmpl << "\")";
    }

    class ValuaScriptException : public std::exception
    {
    private:
        ValuascriptErrorCategory category_;
        ValuascriptErrorCode code_;
        SourceSpan span_;
        std::string message_;

    public:
        ValuaScriptException(ValuascriptErrorCategory cat, ValuascriptErrorCode code, SourceSpan span, std::string msg)
            : category_(cat), code_(code), span_(std::move(span)), message_(std::move(msg))
        {
        }

        [[nodiscard]] const char* what() const noexcept override
        {
            return message_.c_str();
        }

        [[nodiscard]] ValuascriptErrorCategory get_category() const { return category_; }
        [[nodiscard]] const ValuascriptErrorCode& get_code() const { return code_; }
        [[nodiscard]] const SourceSpan& get_span() const { return span_; }

        [[nodiscard]] int get_error_number() const
        {
            return std::visit([](auto&& c) { return static_cast<int>(c); }, code_);
        }

        template <typename T>
        [[nodiscard]] bool is_error(T expected_code) const
        {
            const T* val = std::get_if<T>(&code_);
            return val != nullptr && *val == expected_code;
        }
    };
}
