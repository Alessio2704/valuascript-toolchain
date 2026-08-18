#pragma once
#include <string>
#include <vector>
#include "invalid_modifier_constructs.h"

namespace valuascript::compiler::test
{
    struct ModifierVariant
    {
        std::string name;
        std::string prefix;
    };

    inline const std::vector<ModifierVariant>& get_modifier_variants()
    {
        static const std::vector<ModifierVariant> variants = {
            {.name = "SingleSimple", .prefix = "@test"},
            {.name = "WithArguments", .prefix = "@meta(a: 1)"},
            {.name = "Multiple", .prefix = "@first @second"}
        };
        return variants;
    }

    struct InvalidModifierPlacementTestCase
    {
        std::string test_name;
        InvalidModifierConstructCase construct_case;
        ModifierVariant modifier;
    };

    inline std::vector<InvalidModifierPlacementTestCase> GenerateInvalidModifierTestCases()
    {
        std::vector<InvalidModifierPlacementTestCase> cases;
        const auto& constructs = InvalidModifierConstructRegistry::cases();
        const auto& modifiers = get_modifier_variants();

        for (const auto& construct : constructs)
        {
            for (const auto& mod : modifiers)
            {
                cases.push_back({
                    .test_name = construct.name + "_" + mod.name,
                    .construct_case = construct,
                    .modifier = mod
                });
            }
        }
        return cases;
    }

    inline std::string build_invalid_modifier_snippet(const InvalidModifierPlacementTestCase& test_case)
    {
        return test_case.modifier.prefix + " " + test_case.construct_case.code;
    }
}
