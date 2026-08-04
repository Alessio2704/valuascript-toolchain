#include <gtest/gtest.h>
#include "frontend/parser/helpers/deterministic_sampler.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "invalid_modifier_placement_shared.h"

namespace valuascript::compiler::test
{
#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    class InvalidModifierPlacementDebugger : public ParserTestBase,
                                             public testing::WithParamInterface<InvalidModifierPlacementTestCase>
    {
    };

    TEST_P(InvalidModifierPlacementDebugger, GenerateExpansionReport)
    {
        const auto& test_case = GetParam();
        SCOPED_TRACE("Debugging Invalid Modifier Placement: " + test_case.test_name);

        std::string snippet = build_invalid_modifier_snippet(test_case);
        size_t base_seed = DeterministicSampler::make_seed(test_case.test_name);

        DumpWriter writer(test_case.test_name + ".txt", "invalid_modifier_dumps");
        ASSERT_TRUE(writer.is_open()) << "Failed to open file for writing: " << writer.path_string();

        auto& out = writer.out();
        out << "============================================================\n";
        out << "INVALID MODIFIER EXPANSION DUMP FOR: " << test_case.test_name << "\n";
        out << "Construct: " << test_case.construct_case.name << "\n";
        out << "Modifier:  " << test_case.modifier.name << " (" << test_case.modifier.prefix << ")\n";
        out << "Snippet:   " << snippet << "\n";
        out << "============================================================\n\n";

        size_t variation_index = 0;
        expand_to_top_level_stream(
            test_case.construct_case.type,
            snippet,
            test_case.construct_case.verifier,
            test_case.test_name,
            [&](ProcessingItem&& processed)
            {
                ProgramSpec inner_spec;
                std::visit([&](auto&& ver) { SpecAdder::add(inner_spec, ver); }, processed.verifier);

                auto prog = BuildRecoveryProgram(processed, inner_spec, base_seed + (variation_index * 2));

                variation_index++;
                out << "--- VARIATION " << variation_index << " ---\n";
                out << "PATH:  " << processed.path_name << "\n";
                out << "DEPTH: " << processed.depth << "\n";
                out << "FULL CODE:\n";
                out << prog.full_code;
                out << "------------------------------------------------------------\n\n";
            },
            true,
            test_case.construct_case.skip_contexts
        );

        out << "[DEBUG] Invalid modifier expansion dump finished (" << variation_index << " variations)\n";
    }

    INSTANTIATE_TEST_SUITE_P(
        InvalidModifierPlacementDebug,
        InvalidModifierPlacementDebugger,
        testing::ValuesIn(GenerateInvalidModifierTestCases()),
        TestNameGenerator{}
    );
#endif
}
