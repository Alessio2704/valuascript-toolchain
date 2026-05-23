#include "project_resolver_error_code.h"

namespace valuascript::compiler
{
    std::string_view get_error_template(ProjectResolverErrorCode code)
    {
        switch (code)
        {
        case ProjectResolverErrorCode::CircularImportDetected:
            return "Import Error: Circular import detected involving '{}'.";
        case ProjectResolverErrorCode::ImportFileNotFound:
            return "Import Error: Cannot find module '{}'.";
        }
        return "Unknown Import Error";
    }
}
