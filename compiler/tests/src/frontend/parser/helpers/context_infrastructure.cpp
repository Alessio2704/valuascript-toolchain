#include "context_infrastructure.h"

namespace valuascript::compiler::test
{
    std::string_view get_injectable_type_keyword(InjectableType type)
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

    bool is_valid_declaration_keyword(InjectableType type, const std::string& code)
    {
        std::string_view kw = get_injectable_type_keyword(type);
        if (kw.empty()) return true;
        return code.rfind(kw, 0) == 0;
    }

    bool has_unclosed_brace(const std::string& code)
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
