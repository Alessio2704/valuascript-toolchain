#include "frontend/parser/helpers/parser_test_base.h"
#include "synthetic_generator.h"
#include "utils/memory/arena.h"
#include <string>

namespace valuascript::compiler::test
{
    class SyntheticStressTest : public ParserTestBase
    {
    };

    TEST_F(SyntheticStressTest, CombinatorialFuzzing)
    {
        constexpr size_t iterations = FUZZ_ITERATIONS;
        valuascript::shared::ArenaScope arena_scope(512 * 1024);

        for (size_t seed = 0; seed < iterations; ++seed)
        {
            arena_scope.reset();
            SCOPED_TRACE("Combinatorial Fuzzing Iteration Seed: " + std::to_string(seed));
            SyntheticGenerator gen(seed);
            auto [code, spec] = gen.generate_program(100);
            ExpectValidParse(code, spec);
        }
    }

    TEST_F(SyntheticStressTest, LargeScaleStability)
    {
        SCOPED_TRACE("Large Scale Stability Test (10000 constructs)");
        SyntheticGenerator gen(999);
        auto [code, spec] = gen.generate_program(10000);

        ExpectValidParse(code, spec);
    }

    TEST_F(SyntheticStressTest, DeepRecursionStability)
    {
        SCOPED_TRACE("Deep Recursion Stability Test (Depth 100)");

        std::string expr_code = "1";
        ExprVerifier expr_ver = IsNumber("1");
        for (int i = 0; i < 100; ++i)
        {
            expr_code = "(" + expr_code + " + 1)";
            expr_ver = IsGrouping(IsBinary(TokenType::Plus, expr_ver, IsNumber("1")));
        }

        std::string code = "let deep_val = " + expr_code + "\n";
        ProgramSpec spec;
        spec.execution_steps.emplace_back(IsAssignment({AssignmentTargetSpec{.name = "deep_val"}}, expr_ver));

        ExpectValidParse(code, spec);
    }
}
