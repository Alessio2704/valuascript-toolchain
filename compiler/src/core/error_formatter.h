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
        if constexpr (sizeof...(Args) == 0)
        {
            return std::string(msg_template);
        }
        else
        {
            return std::vformat(msg_template, std::make_format_args(args...));
        }
    }
}
