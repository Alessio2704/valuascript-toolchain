#pragma once
#include <string_view>

namespace valuascript::compiler
{
    enum class FileReaderErrorCode
    {
        FileNotFound = 1001
    };

    std::string_view get_error_template(FileReaderErrorCode code);
}
