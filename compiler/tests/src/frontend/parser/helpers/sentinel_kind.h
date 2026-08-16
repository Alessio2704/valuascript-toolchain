#pragma once

#include <array>
#include <optional>
#include <ostream>
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

    inline std::string format_sentinel_description(std::optional<SentinelKind> kind, bool is_modified)
    {
        if (!kind.has_value()) return "None";
        std::string res = to_string(*kind);
        res += is_modified ? " (Modified)" : " (Unmodified)";
        return res;
    }

    inline void print_sentinel_debug_info(std::ostream& out,
                                          std::optional<SentinelKind> pre_kind, bool is_pre_modified,
                                          std::optional<SentinelKind> post_kind, bool is_post_modified,
                                          std::optional<SentinelKind> inner_pre_kind = std::nullopt, bool is_inner_pre_modified = false,
                                          std::optional<SentinelKind> inner_post_kind = std::nullopt, bool is_inner_post_modified = false)
    {
        if (inner_pre_kind.has_value() || inner_post_kind.has_value())
        {
            out << "OUTER PRE-SENTINEL:  " << format_sentinel_description(pre_kind, is_pre_modified) << "\n";
            out << "INNER PRE-SENTINEL:  " << format_sentinel_description(inner_pre_kind, is_inner_pre_modified) << "\n";
            out << "INNER POST-SENTINEL: " << format_sentinel_description(inner_post_kind, is_inner_post_modified) << "\n";
            out << "OUTER POST-SENTINEL: " << format_sentinel_description(post_kind, is_post_modified) << "\n";
        }
        else
        {
            out << "OUTER PRE-SENTINEL:  " << format_sentinel_description(pre_kind, is_pre_modified) << "\n";
            out << "OUTER POST-SENTINEL: " << format_sentinel_description(post_kind, is_post_modified) << "\n";
        }
    }
}


