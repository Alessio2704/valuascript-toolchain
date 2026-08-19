#pragma once
#include <string_view>

namespace valuascript::compiler
{
    enum class ProjectResolverErrorCode
    {
        CircularImportDetected = 4001,
        ImportFileNotFound
    };

    std::string_view get_error_template(ProjectResolverErrorCode code);
}
