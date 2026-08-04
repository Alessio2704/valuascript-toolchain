#pragma once

#include "frontend/parser/helpers/deterministic_sampler.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/context_names.h"
#include <string>
#include <vector>
#include <string_view>

namespace valuascript::compiler::test
{
    class ExpansionDebugHelper : public ParserTestBase
    {
    public:
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
            DumpWriter writer("expansion_sentinel_recovery_debug_" + label + ".txt", "expansion_dumps");
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

                auto prog = BuildRecoveryProgram(item, base_seed + (scenario_index * 2));

                scenario_index++;

                out << "--- VARIATION " << scenario_index << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "FULL CODE:\n";
                out << prog.full_code;
                out << "------------------------------------------------------------\n\n";
            }, true, skip_contexts, context_overrides, std::nullopt, excluded_sentinels, accepted_sentinels);

            out << "[DEBUG] Recovery expansion dump finished (" << scenario_index << " variations)";
        }
    };
}
