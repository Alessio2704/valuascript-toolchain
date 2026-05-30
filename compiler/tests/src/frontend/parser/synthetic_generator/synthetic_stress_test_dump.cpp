#include "frontend/parser/helpers/parser_test_base.h"
#include "synthetic_generator.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <fstream>
#include <iostream>

namespace valuascript::compiler::test
{
    class SyntheticStressTest : public ParserTestBase
    {
    };

    TEST_F(SyntheticStressTest, InspectGeneratedPrograms)
    {
        std::filesystem::path dump_dir = std::filesystem::current_path() / "fuzz_dumps";
        std::filesystem::create_directories(dump_dir);

        std::cout << "\n======================================================\n";
        std::cout << "Dumping synthetic programs to: \n" << dump_dir.string() << "\n";
        std::cout << "======================================================\n\n";

        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed);
            auto [code, spec] = gen.generate_program(1000);

            DumpWriter writer("program_seed_" + std::to_string(seed) + ".vs");

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

        std::filesystem::path dump_dir = std::filesystem::current_path() / "fuzz_dumps";
        std::filesystem::create_directories(dump_dir);

        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed, cfg);
            auto [code, spec] = gen.generate_program(1000);

            std::filesystem::path file_path = dump_dir / ("experiment_seed_" + std::to_string(seed) + ".vs");
            std::ofstream out(file_path);

            ASSERT_TRUE(out.is_open()) << "Failed to open file for writing: " << file_path;

            out << cfg.generate_report(seed);
            out << gen.get_stats().dump_report(seed);
            out << code;
            out.close();
        }
    }
}
