#include "testing_framework_helpers.h"
#include <vector>
#include <random>

#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/expansion_and_sentinels/context_tree_walker.h"
#include "frontend/parser/expansion_and_sentinels/expansion_policy.h"

namespace valuascript::compiler::test
{
    class ContextTreeWalkerTest : public TestingFrameworkTestBase,
                                  public testing::WithParamInterface<TestingFrameworkSample>
    {
    };

    TEST_F(ContextTreeWalkerTest, RegistryTypeCoverage)
    {
        auto intermediates = GetIntermediateInjectableTypes();
        auto samples = GetTestingFrameworkSamples();
        for (auto type : intermediates)
        {
            bool covered = std::any_of(samples.begin(), samples.end(),
                                       [&](auto& s) { return s.start_type == type; });

            EXPECT_TRUE(covered) << "Logic Gap: Registry defines contexts for type "
                                 << static_cast<int>(type) << " but suite has no sample.";
        }
    }

    TEST_P(ContextTreeWalkerTest, VerifyExhaustiveExpansionAndTopLevelReachability)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        std::vector<ProcessingItem> results;
        expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
        {
            results.push_back(std::move(item));
        }, false);

        ASSERT_FALSE(results.empty()) << "Expansion logic produced zero terminal programs for " << test_name;

        bool top_level_reached = false;
        int max_observed_depth = 0;

        for (const auto& item : results)
        {
            EXPECT_TRUE(is_terminal_type(item.type))
                << "Logic Error: Intermediate type " << static_cast<int>(item.type)
                << " leaked into final results via path: " << item.path_name;

            EXPECT_EQ(CountTransitions(item), static_cast<size_t>(item.depth))
                << "Logic Error: Depth tracking mismatch in path: " << item.path_name;

            EXPECT_LE(item.depth, ExpansionPolicy::current().max_depth + 1);

            if (item.depth > max_observed_depth) max_observed_depth = item.depth;

            if (item.type == InjectableType::TopLevel)
            {
                top_level_reached = true;
            }
        }

        if (!is_terminal_type(start_type))
        {
            EXPECT_GE(max_observed_depth, 1) << "Logic Error: Snippet was never wrapped.";
        }

        if (start_type == InjectableType::StrongStatement || start_type == InjectableType::Expression)
        {
            EXPECT_TRUE(top_level_reached) << "Logic Error: TopLevel context was never reached for " << test_name;
        }

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        for (const auto& ctx : available_contexts)
        {
            bool context_used = std::any_of(results.begin(), results.end(), [&](const auto& res)
            {
                return has_context_segment(res, ctx.name);
            });
            EXPECT_TRUE(context_used) << "Logic Gap: Context '" << ctx.name
                                      << "' is valid for " << test_name << " but was never exercised.";
        }
    }

    TEST_P(ContextTreeWalkerTest, RespectsDepthAndRecursionPolicyOverrides)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        ExpansionPolicy custom_policy{.max_depth = 3, .max_recursion = 1};

        std::vector<ProcessingItem> results;
        expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
        {
            results.push_back(std::move(item));
        }, false, {}, {}, custom_policy);

        for (const auto& item : results)
        {
            EXPECT_LE(item.depth, custom_policy.max_depth + 1)
                << "Item depth " << item.depth << " exceeded max_depth + 1 limit of "
                << (custom_policy.max_depth + 1) << " in path: " << item.path_name;

            EXPECT_LE(item.recursion_depth, custom_policy.max_recursion)
                << "Item recursion depth " << item.recursion_depth << " exceeded max_recursion limit of "
                << custom_policy.max_recursion << " in path: " << item.path_name;
        }
    }

    TEST_P(ContextTreeWalkerTest, VerifyRandomWalkStrategyTerminates)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        struct WalkState
        {
            InjectableType type;
            int depth;
        };

        ContextTreeWalker<WalkState>::Callbacks cb;
        cb.get_type = [](const WalkState& s) { return s.type; };

        bool reached_terminal = false;
        cb.on_terminal = [&](WalkState) { reached_terminal = true; };

        cb.on_normal_branch = [](const WalkState& s, const Context& ctx, int) -> std::vector<WalkState>
        {
            return { WalkState{.type = ctx.output_type, .depth = s.depth + 1} };
        };

        cb.on_block_branch = [](const WalkState& s, const Context& ctx, int) -> std::vector<WalkState>
        {
            return { WalkState{.type = ctx.output_type, .depth = s.depth + 1} };
        };

        std::mt19937 rng(42);
        cb.strategy = [&](const std::vector<NextStep>& steps, int, int) -> std::vector<NextStep>
        {
            if (steps.empty()) return {};
            std::uniform_int_distribution<size_t> dist(0, steps.size() - 1);
            return {steps[dist(rng)]};
        };

        bool ever_reached = false;
        for (int i = 0; i < 50; ++i)
        {
            reached_terminal = false;
            ContextTreeWalker<WalkState>::walk(WalkState{.type = start_type, .depth = 0}, 0, 0, cb, ExpansionPolicy{.max_depth = 20, .max_recursion = 5});
            if (reached_terminal)
            {
                ever_reached = true;
                break;
            }
        }

        EXPECT_TRUE(ever_reached) << "Random Walk Strategy failed to ever reach TopLevel after 50 attempts!";
    }

    INSTANTIATE_TEST_SUITE_P(
        ContextTreeWalkerTests,
        ContextTreeWalkerTest,
        testing::ValuesIn(GetTestingFrameworkSamples()),
        TestNameGenerator{}
    );
}
