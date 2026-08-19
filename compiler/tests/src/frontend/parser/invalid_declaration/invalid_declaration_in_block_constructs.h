#pragma once

#include "invalid_declaration_in_block_shared.h"

namespace valuascript::compiler::test
{
    class InvalidDeclarationConstructRegistry
    {
    public:
        static const std::vector<InvalidDeclarationConstructCase>& cases();
        static std::vector<InvalidDeclarationConstructCase> cases_for_context(const Context& ctx);
    };
}
