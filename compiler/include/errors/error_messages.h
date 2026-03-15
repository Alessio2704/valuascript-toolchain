#pragma once

#include <string_view>
#include <string>
#include <format>
#include "errors/valuascript_exception.h"

namespace valuascript::compiler {

    std::string_view get_error_template(ErrorCode code);

    template <typename... Args>
    std::string format_error_message(ErrorCode code, Args&&... args) {
        std::string_view msg_template = get_error_template(code);
        return std::vformat(msg_template, std::make_format_args(args...));
    }

}