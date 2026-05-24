#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/synthetic_generator.h"
#include "utils/test_env_config.h"

namespace valuascript::compiler::test
{
    class SyntheticStressTest : public ParserTestBase
    {
    };

    TEST_F(SyntheticStressTest, CombinatorialFuzzing)
    {
        constexpr size_t iterations = FUZZ_ITERATIONS;

        for (size_t seed = 0; seed < iterations; ++seed)
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
        auto [expr_code, expr_ver] = gen.synth_expression(0, 100);

        std::string code = "let deep_val = " + expr_code;
        ProgramSpec spec;
        spec.execution_steps.emplace_back(IsAssignment({{{}, "deep_val"}}, expr_ver));
        ExpectValidParse(code, spec);
    }
}
