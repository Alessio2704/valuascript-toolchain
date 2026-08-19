#include "frontend/parser/helpers/parser_test_base.h"
#include "synthetic_generator.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <fstream>

namespace valuascript::compiler::test
{
    class SyntheticStressTest : public ParserTestBase
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(SyntheticStressTest, InspectGeneratedPrograms)
    {
        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed);
            auto [code, spec] = gen.generate_program(1000);

            DumpWriter writer("program_seed_" + std::to_string(seed) + ".vs", "synthetic_generator_dumps");

            ASSERT_TRUE(writer.out().is_open()) << "Failed to open file for writing: " << writer.path_string();

            writer.out() << "// ==========================================\n";
            writer.out() << "// Synthetic Program Dump (Seed: " << seed << ")\n";
            writer.out() << "// ==========================================\n\n";
            writer.out() << gen.get_stats().dump_report(seed);
            writer.out() << code;
        }
    }

    TEST_F(SyntheticStressTest, InspectGeneratedProgramsWithConfig)
    {
        SyntheticGeneratorConfig cfg;

        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed, cfg);
            auto [code, spec] = gen.generate_program(1000);

            DumpWriter writer("experiment_seed_" + std::to_string(seed) + ".vs", "synthetic_generator_dumps");

            ASSERT_TRUE(writer.out().is_open()) << "Failed to open file for writing: " << writer.path_string();

            auto& out = writer.out();
            out << cfg.generate_report(seed);
            out << gen.get_stats().dump_report(seed);
            out << code;
        }
    }
#endif
}
