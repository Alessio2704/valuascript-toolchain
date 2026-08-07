#pragma once

#include <string>
#include <vector>
#include <string_view>
#include "frontend/parser/helpers/injectable_type.h"
#include "frontend/parser/helpers/universal_verifier.h"
#include "frontend/parser/helpers/node_matchers.h"

namespace valuascript::compiler::test
{
    struct InvalidModifierConstructCase
    {
        std::string name;
        std::string code;
        UniversalVerifier verifier;
        InjectableType type = InjectableType::WeakStatement;
        std::vector<std::string_view> skip_contexts = {};
    };

    class InvalidModifierConstructRegistry
    {
    public:
        static const std::vector<InvalidModifierConstructCase>& cases();
        static void add(InvalidModifierConstructCase spec);
    };
}
