#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace valuascript::compiler::test
{
    enum class InjectableType
    {
        Identifier,
        Import, Directive, Function, Extension, Struct, Enum, TypeAlias,
        Expression, Modifier, TypeAnnotation,
        WeakStatement, StrongStatement,
        TopLevel
    };

    namespace InjectableTypes
    {
        inline constexpr std::array<InjectableType, 14> All = {
            InjectableType::Identifier,
            InjectableType::Import,
            InjectableType::Directive,
            InjectableType::Function,
            InjectableType::Extension,
            InjectableType::Struct,
            InjectableType::Enum,
            InjectableType::TypeAlias,
            InjectableType::Expression,
            InjectableType::Modifier,
            InjectableType::TypeAnnotation,
            InjectableType::WeakStatement,
            InjectableType::StrongStatement,
            InjectableType::TopLevel
        };

        inline std::vector<InjectableType> all()
        {
            return {All.begin(), All.end()};
        }
    }

    inline constexpr bool is_terminal_type(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::TopLevel:
        case InjectableType::Import:
        case InjectableType::Directive:
        case InjectableType::Function:
        case InjectableType::Extension:
        case InjectableType::Struct:
        case InjectableType::Enum:
        case InjectableType::TypeAlias:
            return true;
        default:
            return false;
        }
    }

    inline constexpr bool is_intermediate_type(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::Expression:
        case InjectableType::TypeAnnotation:
        case InjectableType::Modifier:
        case InjectableType::StrongStatement:
        case InjectableType::WeakStatement:
            return true;
        default:
            return false;
        }
    }

    inline std::vector<InjectableType> get_intermediate_injectable_types()
    {
        std::vector<InjectableType> intermediates;
        for (auto type : InjectableTypes::All)
        {
            if (is_intermediate_type(type))
            {
                intermediates.push_back(type);
            }
        }
        return intermediates;
    }

    inline std::string_view to_string(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::Identifier: return "Identifier";
        case InjectableType::Import: return "Import";
        case InjectableType::Directive: return "Directive";
        case InjectableType::Function: return "Function";
        case InjectableType::Extension: return "Extension";
        case InjectableType::Struct: return "Struct";
        case InjectableType::Enum: return "Enum";
        case InjectableType::TypeAlias: return "TypeAlias";
        case InjectableType::Expression: return "Expression";
        case InjectableType::Modifier: return "Modifier";
        case InjectableType::TypeAnnotation: return "TypeAnnotation";
        case InjectableType::WeakStatement: return "WeakStatement";
        case InjectableType::StrongStatement: return "StrongStatement";
        case InjectableType::TopLevel: return "TopLevel";
        default: return "Unknown";
        }
    }

    inline std::string_view get_injectable_type_keyword(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::Import: return "import";
        case InjectableType::Directive: return "#";
        case InjectableType::Struct: return "struct";
        case InjectableType::Enum: return "enum";
        case InjectableType::Extension: return "extension";
        case InjectableType::TypeAlias: return "typealias";
        default: return "";
        }
    }

    inline bool is_valid_declaration_keyword(InjectableType type, const std::string& code)
    {
        std::string_view kw = get_injectable_type_keyword(type);
        if (kw.empty()) return true;
        return code.rfind(kw, 0) == 0;
    }

    inline bool has_unclosed_brace(const std::string& code)
    {
        int balance = 0;
        for (char c : code)
        {
            if (c == '{') balance++;
            else if (c == '}') balance--;
        }
        return balance != 0;
    }
}
