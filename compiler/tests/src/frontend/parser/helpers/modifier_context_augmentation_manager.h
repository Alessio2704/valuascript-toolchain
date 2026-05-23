#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "test_structures.h"
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    class ModifierContextAugmentationManager
    {
    public:
        static std::vector<ProcessingItem> generate_variations(const ProcessingItem& initial_item)
        {
            std::vector<ProcessingItem> items;

            items.push_back(initial_item);

            auto base_v = std::get<ModifierVerifier>(initial_item.verifier);

            struct ModifierAtom
            {
                std::string code;
                std::vector<ModifierSpec> specs;
                bool is_target;
            };

            std::vector<ModifierAtom> atoms = {
                {initial_item.code, base_v, true},
                {"@mod1", {{"mod1", {}}}, false},
                {"@mod2()", {{"mod2", {}}}, false},
                {"@mod3(p: 1)", {{"mod3", {{"p", IsNumber("1")}}}}, false}
            };

            std::vector<int> indices = {0, 1, 2, 3};
            std::sort(indices.begin(), indices.end());

            do
            {
                ProcessingItem var = initial_item;
                std::string full_code;
                std::string prefix_for_target;
                ModifierVerifier merged_verifier;

                for (size_t i = 0; i < indices.size(); ++i)
                {
                    if (i > 0) full_code += " ";
                    const auto& atom = atoms[static_cast<size_t>(indices[i])];

                    if (atom.is_target)
                    {
                        prefix_for_target = full_code + initial_item.cumulative_prefix;
                    }

                    full_code += atom.code;
                    merged_verifier.insert(merged_verifier.end(), atom.specs.begin(), atom.specs.end());
                }

                var.code = full_code;
                var.cumulative_prefix = prefix_for_target;
                var.verifier = merged_verifier;

                std::string path_desc = "Stacked [";
                for (size_t k = 0; k < indices.size(); ++k)
                {
                    path_desc += (atoms[k].is_target
                                      ? "Target"
                                      : "Mod" + std::to_string(indices[k]));
                    if (k < indices.size() - 1) path_desc += ", ";
                }
                path_desc += "]";

                var.path_name = initial_item.path_name + " (" + path_desc + ")";
                items.push_back(std::move(var));
            }
            while (std::next_permutation(indices.begin(), indices.end()));

            return items;
        }
    };
}
