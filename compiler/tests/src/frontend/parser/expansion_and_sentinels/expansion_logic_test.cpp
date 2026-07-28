#include "frontend/parser/helpers/parser_test_base.h"
#include <set>
#include <vector>
#include <random>

#include "frontend/parser/helpers/context_registry.h"
#include "context_tree_walker.h"
#include "expansion_policy.h"
#include "frontend/parser/helpers/recovery_sentinel.h"

namespace valuascript::compiler::test
{
    struct IntegritySample
    {
        InjectableType start_type;
        std::string snippet;
        std::string test_name;
    };

    static std::vector<IntegritySample> GetIntegritySamples()
    {
        return {
            {InjectableType::Expression, "1", "Expression"},
            {InjectableType::TypeAnnotation, "int", "TypeAnnotation"},
            {InjectableType::Modifier, "@meta", "Modifier"},
            {InjectableType::StrongStatement, "let x = 1", "StrongStatement"},
            {InjectableType::WeakStatement, "return 1", "WeakStatement"}
        };
    }

    class ContextExpansionFrameworkIntegrityTest : public ParserTestBase,
                                                   public testing::WithParamInterface<IntegritySample>
    {
    protected:
        static size_t CountTransitions(const std::string& path)
        {
            size_t count = 0;
            size_t pos = path.find(" -> ");
            while (pos != std::string::npos)
            {
                count++;
                pos = path.find(" -> ", pos + 4);
            }
            return count;
        }
    };

    TEST_F(ContextExpansionFrameworkIntegrityTest, RegistryTypeCoverage)
    {
        std::vector<InjectableType> intermediates = {
            InjectableType::Expression, InjectableType::TypeAnnotation,
            InjectableType::Modifier, InjectableType::StrongStatement,
            InjectableType::WeakStatement
        };

        auto samples = GetIntegritySamples();
        for (auto type : intermediates)
        {
            bool covered = std::any_of(samples.begin(), samples.end(),
                                       [&](auto& s) { return s.start_type == type; });

            EXPECT_TRUE(covered) << "Logic Gap: Registry defines contexts for type "
                                 << static_cast<int>(type) << " but suite has no sample.";
        }
    }

    TEST_P(ContextExpansionFrameworkIntegrityTest, VerifyExhaustiveExpansionLogic)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        std::vector<ProcessingItem> results;

        expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
        {
            results.push_back(std::move(item));
        }, false);

        ASSERT_FALSE(results.empty()) << "Expansion logic produced zero terminal programs.";

        bool elevation_occurred = false;
        int max_observed_depth = 0;

        for (const auto& item : results)
        {
            EXPECT_TRUE(is_terminal_type(item.type))
                << "Logic Error: Intermediate type " << static_cast<int>(item.type)
                << " leaked into final results via path: " << item.path_name;

            EXPECT_EQ(CountTransitions(item.path_name), static_cast<size_t>(item.depth))
                << "Logic Error: Depth tracking mismatch in path: " << item.path_name;

            EXPECT_LE(item.depth, ExpansionPolicy::current().max_depth + 1);

            if (item.depth > max_observed_depth) max_observed_depth = item.depth;

            if (item.path_name.find("TopLevelPromotion") != std::string::npos)
            {
                elevation_occurred = true;
                EXPECT_EQ(item.type, InjectableType::TopLevel)
                    << "Logic Error: Elevation occurred but output type is not TopLevel.";
            }
        }

        if (!is_terminal_type(start_type))
        {
            EXPECT_GE(max_observed_depth, 1) << "Logic Error: Snippet was never wrapped.";
        }

        if (start_type == InjectableType::StrongStatement || start_type == InjectableType::Expression)
        {
            EXPECT_TRUE(elevation_occurred) << "Logic Error: Elevation branch (Promotion) was never taken.";
        }

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        for (const auto& ctx : available_contexts)
        {
            bool context_used = std::any_of(results.begin(), results.end(), [&](const auto& res)
            {
                return res.path_name.find(ctx.name) != std::string::npos;
            });
            EXPECT_TRUE(context_used) << "Logic Gap: Context '" << ctx.name
                                      << "' is valid for " << test_name << " but was never exercised.";
        }
    }

    TEST_P(ContextExpansionFrameworkIntegrityTest, VerifyRandomWalkStrategyTerminates)
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

        cb.on_promotion = [&](const WalkState& s)
        {
            cb.on_terminal({InjectableType::TopLevel, s.depth + 1});
        };

        cb.on_normal_branch = [](const WalkState& s, const Context& ctx, int)
        {
            return WalkState{ctx.output_type, s.depth + 1};
        };

        cb.on_block_branch = [](const WalkState& s, const Context& ctx, int)
        {
            return WalkState{ctx.output_type, s.depth + 1};
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
            ContextTreeWalker<WalkState>::walk({start_type, 0}, 0, 0, cb, ExpansionPolicy{20, 5});
            if (reached_terminal)
            {
                ever_reached = true;
                break;
            }
        }

        EXPECT_TRUE(ever_reached) << "Random Walk Strategy failed to ever reach TopLevel after 50 attempts!";
    }

    INSTANTIATE_TEST_SUITE_P(
        ContextExpansionFrameworkIntegrityTest,
        ContextExpansionFrameworkIntegrityTest,
        testing::ValuesIn(GetIntegritySamples()),
        [](const testing::TestParamInfo<IntegritySample>& test_info) { return test_info.param.test_name; }
    );

    class RecoveryExpansionIntegrityTest : public ParserTestBase,
                                           public testing::WithParamInterface<IntegritySample>
    {
    };

    TEST_P(RecoveryExpansionIntegrityTest, VerifySentinelInjectionInAllContexts)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        size_t base_seed = 0x1337;
        size_t scenario_index = 0;

        expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
        {
            SCOPED_TRACE("Checking Scenario: " + item.path_name);

            ProgramSpec item_spec;
            auto prog = BuildRecoveryProgram(
                std::move(item.code),
                std::move(item_spec),
                std::move(item.cumulative_prefix),
                base_seed + (scenario_index++ * 2)
            );

            CompilerContext context;

            auto ast = run_parser(prog.full_code, context);
            ASSERT_NE(ast, nullptr) << "Failed to parse generated recovery code:\n" << prog.full_code;

            size_t total_top_level_items =
                ast->import_statements.size() +
                ast->directives.size() +
                ast->function_definitions.size() +
                ast->struct_definitions.size() +
                ast->enum_definitions.size() +
                ast->type_aliases.size() +
                ast->extension_definitions.size() +
                ast->execution_steps.size();

            EXPECT_EQ(total_top_level_items, 3)
                << "Top-level sentinel injection missing in path: " << item.path_name;

            if (item.path_name.find("function_body_wrapper") != std::string::npos)
            {
                bool wrapper_found = false;
                for (const auto& f : ast->function_definitions)
                {
                    if (f->name == "ctx_wrapper")
                    {
                        wrapper_found = true;
                        EXPECT_EQ(f->body.size(), 3)
                            << "Block-level sentinels missing inside ctx_wrapper for path: " << item.path_name
                            << "\nBody size was: " << f->body.size();
                    }
                }
                EXPECT_TRUE(wrapper_found) << "Path indicated a function wrapper, but ctx_wrapper not found in AST.";
            }
            else if (item.path_name.find("extension_body_wrapper") != std::string::npos)
            {
                bool wrapper_found = false;
                for (const auto& e : ast->extension_definitions)
                {
                    wrapper_found = true;
                    size_t body_size = e->execution_steps.size() + e->function_definitions.size() + e->struct_definitions.size() + e->enum_definitions.size() + e->type_aliases.size();
                    EXPECT_EQ(body_size, 3)
                        << "Block-level sentinels missing inside extension_body_wrapper for path: " << item.path_name
                        << "\nBody size was: " << body_size;
                }
                EXPECT_TRUE(wrapper_found) << "Path indicated an extension wrapper, but no extension found in AST.";
            }
        }, true);
    }

    INSTANTIATE_TEST_SUITE_P(
        RecoveryIntegrity,
        RecoveryExpansionIntegrityTest,
        testing::ValuesIn(GetIntegritySamples()),
        [](const testing::TestParamInfo<IntegritySample>& test_info) { return test_info.param.test_name; }
    );
}
