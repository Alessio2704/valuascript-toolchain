#pragma once

#include <array>
#include <string>
#include <vector>

namespace valuascript::compiler::test
{
    enum class SentinelKind
    {
        Assignment,
        Reassignment,
        ExprStmt,
        Return,
        Import,
        Function,
        Enum,
        Alias,
        Directive,
        Struct
    };

    namespace SentinelKinds
    {
        inline constexpr std::array<SentinelKind, 10> All = {
            SentinelKind::Assignment,
            SentinelKind::Reassignment,
            SentinelKind::ExprStmt,
            SentinelKind::Return,
            SentinelKind::Import,
            SentinelKind::Function,
            SentinelKind::Enum,
            SentinelKind::Alias,
            SentinelKind::Directive,
            SentinelKind::Struct
        };

        inline std::vector<SentinelKind> all()
        {
            return {All.begin(), All.end()};
        }
    }

    inline std::string to_string(SentinelKind kind)
    {
        switch (kind)
        {
        case SentinelKind::Assignment: return "Assignment";
        case SentinelKind::Reassignment: return "Reassignment";
        case SentinelKind::ExprStmt: return "ExprStmt";
        case SentinelKind::Return: return "Return";
        case SentinelKind::Import: return "Import";
        case SentinelKind::Function: return "Function";
        case SentinelKind::Enum: return "Enum";
        case SentinelKind::Alias: return "Alias";
        case SentinelKind::Directive: return "Directive";
        case SentinelKind::Struct: return "Struct";
        }
        return "Unknown";
    }
}
