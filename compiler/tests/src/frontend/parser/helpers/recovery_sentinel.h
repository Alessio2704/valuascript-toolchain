#pragma once
#include "language_registry.h"
#include <algorithm>

namespace valuascript::compiler::test {
    struct RecoveryBlock {
        std::string source;
        ProgramSpec spec;
    };

    class RecoverySentinel {
    public:
        static RecoveryBlock generate(size_t rotation_index) {
            auto constructs = LanguageRegistry::all();

            if (!constructs.empty()) {
                std::rotate(constructs.begin(),
                            constructs.begin() + (rotation_index % constructs.size()),
                            constructs.end());
            }

            std::string full_source = "\n// --- RECOVERY SENTINEL ---\n";
            ProgramSpec full_spec;

            for (const auto &c: constructs) {
                full_source += c.source;
                c.add_to_spec(full_spec);
            }

            return {std::move(full_source), std::move(full_spec)};
        }
    };
}
