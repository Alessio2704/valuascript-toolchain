#pragma once

#include "frontend/parser/helpers/deterministic_sampler.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/construct_registry.h"
#include <string>
#include <vector>
#include <string_view>

namespace valuascript::compiler::test
{
    struct DumpTest
    {
        std::string_view name;
    };

    class ExpansionDebugHelper : public ParserTestBase
    {
    public:
        static void DebugRecovery(std::string_view test_name)
        {
            bool found = ErrorRegistry::find(test_name, [](InjectableType type, const auto& entry) {
                DumpRecoveryExpansion(
                    type,
                    entry.code,
                    entry.test_name,
                    entry.skip_contexts,
                    to_any_overrides(entry.context_overrides),
                    entry.excluded_sentinels,
                    entry.accepted_sentinels
                );
            });
            if (!found)
            {
                ADD_FAILURE() << "DebugRecovery failed: Test case '" << test_name << "' not found in ErrorRegistry.";
            }
        }

        static void DebugFeature(std::string_view test_name)
        {
            bool found = ConstructRegistry::find(test_name, [](InjectableType type, const auto& entry) {
                DumpFeatureExpansion(
                    type,
                    entry.code,
                    entry.test_name,
                    entry.skip_contexts
                );
            });
            if (!found)
            {
                ADD_FAILURE() << "DebugFeature failed: Test case '" << test_name << "' not found in ConstructRegistry.";
            }
        }
        static void DumpFeatureExpansion(InjectableType type,
                                         const std::string& snippet,
                                         const std::string& label,
                                         const std::vector<std::string_view>& skip_contexts = {},
                                         const std::vector<ContextOverrideAny>& context_overrides = {},
                                         const std::vector<SentinelKind>& excluded_sentinels = {},
                                         const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            DumpWriter writer("expansion_features_debug_" + label + ".txt", "expansion_dumps");
            if (!writer.is_open()) return;
            auto& out = writer.out();

            out << "============================================================\n";
            out << "EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            size_t count = 0;
            expand_to_top_level_stream(type, snippet, NullVerifier{}, label, [&](ProcessingItem&& item)
            {
                count++;
                out << "--- VARIATION " << count << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "CODE:\n" << item.code << "\n";
                out << "------------------------------------------------------------\n\n";
            }, false, skip_contexts, context_overrides, std::nullopt, excluded_sentinels, accepted_sentinels);

            out << "[DEBUG] Feature expansion dump finished (" << count << " variations)";
        }

        static void DumpRecoveryExpansion(InjectableType type,
                                          const std::string& snippet,
                                          const std::string& label,
                                          const std::vector<std::string_view>& skip_contexts = {},
                                          const std::vector<ContextOverrideAny>& context_overrides = {},
                                          const std::vector<SentinelKind>& excluded_sentinels = {},
                                          const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            DumpWriter writer("expansion_recovery_debug_" + label + ".txt", "expansion_dumps");
            if (!writer.is_open()) return;

            auto& out = writer.out();

            out << "============================================================\n";
            out << "RECOVERY EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            size_t scenario_index = 0;
            size_t base_seed = DeterministicSampler::make_seed(label);

            expand_to_top_level_stream(type, snippet, NullVerifier{}, label, [&](ProcessingItem&& item)
            {
                if (item.is_skipped) return;

                ForEachRecoveryProgram(item, base_seed + (scenario_index * 2), [&](const ConstructedRecoveryProgram& prog)
                {
                    scenario_index++;

                    out << "--- VARIATION " << scenario_index << " ---\n";
                    out << "PATH:  " << (prog.path_name.empty() ? item.path_name : prog.path_name) << "\n";
                    out << "DEPTH: " << item.depth << "\n";
                    out << "FULL CODE:\n";
                    out << prog.full_code;
                    out << "------------------------------------------------------------\n\n";
                });
            }, true, skip_contexts, context_overrides, std::nullopt, excluded_sentinels, accepted_sentinels);

            out << "[DEBUG] Recovery expansion dump finished (" << scenario_index << " variations)";
        }
    };
}
