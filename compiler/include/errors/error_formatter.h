#pragma once
#include <string_view>
#include <format>
#include <concepts>

namespace valuascript::compiler {
    enum class ValuascriptErrorCode;
    enum class InternalErrorCode;

    template<typename T>
    concept CompilerErrorEnum = std::same_as<T, ValuascriptErrorCode> || std::same_as<T, InternalErrorCode>;

    std::string_view get_error_template(ValuascriptErrorCode code);

    std::string_view get_error_template(InternalErrorCode code);

    template<typename... Args>
    std::string format_error(CompilerErrorEnum auto code, Args &&... args) {
        std::string_view msg_template = get_error_template(code);
        return std::vformat(msg_template, std::make_format_args(args...));
    }
}
