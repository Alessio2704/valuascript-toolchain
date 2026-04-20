#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/synthetic_generator.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace valuascript::compiler::test
{
    class SyntheticStressTest : public ParserTestBase
    {
    };

    TEST_F(SyntheticStressTest, CombinatorialFuzzing)
    {
        for (size_t seed = 0; seed < 1000; ++seed)
        {
            SyntheticGenerator gen(seed);
            auto [code, spec] = gen.generate_program(100);
            ExpectValidParse(code, spec);
        }
    }

    TEST_F(SyntheticStressTest, LargeScaleStability)
    {
        SyntheticGenerator gen(999);
        auto [code, spec] = gen.generate_program(10000);

        ExpectValidParse(code, spec);
    }

    TEST_F(SyntheticStressTest, DeepRecursionStability)
    {
        SyntheticGenerator gen(1000);
        auto [expr_code, expr_ver] = gen.synth_expression(0, 50);

        std::string code = "let deep_val = " + expr_code;
        ProgramSpec spec;
        spec.execution_steps.emplace_back(IsAssignment({}, {{"deep_val"}}, expr_ver));
        ExpectValidParse(code, spec);
    }

    TEST_F(SyntheticStressTest, InspectGeneratedPrograms)
    {
        std::filesystem::path dump_dir = std::filesystem::current_path() / "fuzz_dumps";
        std::filesystem::create_directories(dump_dir);

        std::cout << "\n======================================================\n";
        std::cout << "Dumping 5 synthetic programs to: \n" << dump_dir.string() << "\n";
        std::cout << "======================================================\n\n";

        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed);
            auto [code, spec] = gen.generate_program(100);

            std::filesystem::path file_path = dump_dir / ("program_seed_" + std::to_string(seed) + ".vs");
            std::ofstream out(file_path);

            ASSERT_TRUE(out.is_open()) << "Failed to open file for writing: " << file_path;

            out << "// ==========================================\n";
            out << "// Synthetic Program Dump (Seed: " << seed << ")\n";
            out << "// ==========================================\n\n";
            out << code;
            out.close();
        }
    }
}
