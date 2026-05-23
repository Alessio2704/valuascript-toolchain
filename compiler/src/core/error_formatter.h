#pragma once
#include <string_view>
#include <format>
#include <concepts>

namespace valuascript::compiler
{
    template <typename T>
    concept CompilerErrorEnum = requires(T t)
    {
        { get_error_template(t) } -> std::convertible_to<std::string_view>;
    };

    template <CompilerErrorEnum T, typename... Args>
    std::string format_error(T code, Args&&... args)
    {
        std::string_view msg_template = get_error_template(code);
        return std::vformat(msg_template, std::make_format_args(args...));
    }
}
