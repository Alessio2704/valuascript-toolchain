#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "node_matchers.h"
#include "error_registry.h"

namespace valuascript::compiler::test
{
    inline bool is_terminal_type(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::TopLevel:
        case InjectableType::Import:
        case InjectableType::Directive:
        case InjectableType::Function:
        case InjectableType::Struct:
        case InjectableType::Enum:
        case InjectableType::TypeAlias:
            return true;
        default:
            return false;
        }
    }

    struct ProcessingItem
    {
        InjectableType type;
        std::string code;
        UniversalVerifier verifier;
        std::string path_name;
        std::string cumulative_prefix;
        int depth;
        int recursion_depth;
        std::vector<std::string_view> skip_contexts;
        bool is_skipped;
    };

    struct RecoveryScenario
    {
        std::string path_name;
        std::string full_code;
        ProgramSpec spec;
        std::vector<ParserExpectedError> shifted_errors;
        int depth;
    };
}
