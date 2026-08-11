#include "testing_framework_helpers.h"
#include <algorithm>
#include <map>
#include <vector>
#include <string>

#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/recovery_sentinel.h"

#include "frontend/parser/helpers/recovery_program_builder.h"

namespace valuascript::compiler::test
{
    class SentinelInjectionTest : public TestingFrameworkTestBase,
                                 public testing::WithParamInterface<TestingFrameworkSample>
    {
    };

    TEST_P(SentinelInjectionTest, VerifyPreAndPostSentinelInEveryContextBlock)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        size_t base_seed = 0x1337;
        size_t scenario_index = 0;

        expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
        {
            SCOPED_TRACE("Checking Scenario: " + item.path_name);

            ProgramSpec item_spec;
            ForEachRecoveryProgram(
                std::move(item.code),
                std::move(item_spec),
                std::move(item.cumulative_prefix),
                base_seed + (scenario_index++ * 2),
                item.excluded_sentinels,
                item.accepted_sentinels,
                item.path_name,
                [&](const ConstructedRecoveryProgram& prog)
                {
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
                            size_t body_size = e->execution_steps.size() + e->function_definitions.size() +
                                              e->struct_definitions.size() + e->enum_definitions.size() +
                                              e->type_aliases.size();
                            EXPECT_EQ(body_size, 3)
                                << "Block-level sentinels missing inside extension_body_wrapper for path: " << item.path_name
                                << "\nBody size was: " << body_size;
                        }
                        EXPECT_TRUE(wrapper_found) << "Path indicated an extension wrapper, but no extension found in AST.";
                    }
                });
        }, true);
    }

    TEST_F(SentinelInjectionTest, AsymptoticExcludedSentinelsNeverAppearOver1000Seeds)
    {
        std::vector<BlockContext> block_contexts = {
            BlockContext::TopLevel,
            BlockContext::FunctionBody,
            BlockContext::ExtensionBody
        };

        std::vector<SentinelKind> excluded = { SentinelKind::Import, SentinelKind::Return };

        for (auto ctx : block_contexts)
        {
            for (size_t seed = 1; seed <= 1000; ++seed)
            {
                RecoveryBlock block = RecoverySentinel::generate_block_sentinel(seed, ctx, excluded, {});
                if (block.kind.has_value())
                {
                    bool is_excluded = std::find(excluded.begin(), excluded.end(), *block.kind) != excluded.end();
                    EXPECT_FALSE(is_excluded)
                        << "Excluded sentinel kind " << to_string(*block.kind)
                        << " appeared at seed " << seed << " in block context " << static_cast<int>(ctx);
                }
            }
        }
    }

    TEST_F(SentinelInjectionTest, AcceptedSentinelsOnlyAndExhaustiveExpansion)
    {
        std::vector<SentinelKind> accepted = { SentinelKind::Assignment, SentinelKind::Reassignment };

        ProcessingItem base_item{
            .type = InjectableType::Expression, .code = "1", .verifier = NullVerifier{}, .path_name = "TestPath",
            .cumulative_prefix = "", .depth = 0, .recursion_depth = 0,
            .skip_contexts = {}, .is_skipped = false, .context_overrides = {}, .custom_errors = std::nullopt,
            .excluded_sentinels = {}, .accepted_sentinels = accepted
        };

        std::vector<ConstructedRecoveryProgram> progs;
        ForEachRecoveryProgram(base_item, 100, [&](const ConstructedRecoveryProgram& prog)
        {
            progs.push_back(prog);
        });

        EXPECT_EQ(progs.size(), accepted.size())
            << "Accepted sentinels expansion should consecutively produce exactly " << accepted.size() << " variations.";

        for (size_t i = 0; i < progs.size(); ++i)
        {
            EXPECT_NE(progs[i].path_name.find(to_string(accepted[i])), std::string::npos);
        }
    }

    TEST_F(SentinelInjectionTest, EffectiveAcceptedSentinelsExcludesForbiddenKindsAndProducesCorrectVariationCount)
    {
        std::vector<SentinelKind> accepted = SentinelKinds::all();
        std::vector<SentinelKind> excluded = {
            SentinelKind::Assignment,
            SentinelKind::Reassignment,
            SentinelKind::ExprStmt
        };

        std::vector<SentinelKind> expected_effective = {
            SentinelKind::Return,
            SentinelKind::Import,
            SentinelKind::Function,
            SentinelKind::Enum,
            SentinelKind::Alias,
            SentinelKind::Directive,
            SentinelKind::Struct
        };

        std::string snippet = "enum Test: int { A }";
        ProgramSpec spec;

        auto progs = RecoveryProgramBuilder::BuildRecoveryPrograms(
            snippet,
            spec,
            "",
            0x1234,
            excluded,
            accepted,
            "TestEnumPath"
        );

        ASSERT_EQ(progs.size(), expected_effective.size());

        for (size_t i = 0; i < progs.size(); ++i)
        {
            std::string expected_suffix = "[" + to_string(expected_effective[i]) + "]";
            EXPECT_TRUE(progs[i].path_name.ends_with(expected_suffix))
                << "Variation index " << i << " path name '" << progs[i].path_name
                << "' should end with '" << expected_suffix << "'";

            for (auto exc : excluded)
            {
                std::string forbidden_suffix = "[" + to_string(exc) + "]";
                EXPECT_FALSE(progs[i].path_name.ends_with(forbidden_suffix))
                    << "Variation index " << i << " path name '" << progs[i].path_name
                    << "' must not end with forbidden sentinel suffix '" << forbidden_suffix << "'";
            }
        }
    }

    TEST_F(SentinelInjectionTest, AcceptedSentinelsPseudoRandomUniformDistributionChiSquaredTest)
    {
        std::vector<SentinelKind> accepted = {
            SentinelKind::Assignment,
            SentinelKind::Reassignment,
            SentinelKind::ExprStmt
        };

        std::map<SentinelKind, size_t> counts;
        for (auto k : accepted) counts[k] = 0;

        const size_t total_samples = 500;

        for (size_t seed = 1; seed <= total_samples; ++seed)
        {
            SentinelKind chosen_kind = DeterministicSampler::sample_element_rng(accepted, seed);
            RecoveryBlock block = RecoverySentinel::generate_block_sentinel(
                seed * 1000 + 42,
                BlockContext::TopLevel,
                {},
                { chosen_kind }
            );

            ASSERT_TRUE(block.kind.has_value());
            EXPECT_EQ(*block.kind, chosen_kind);
            counts[chosen_kind]++;
        }

        double expected_per_category = static_cast<double>(total_samples) / static_cast<double>(accepted.size());
        double chi_squared = 0.0;

        for (auto k : accepted)
        {
            double observed = static_cast<double>(counts[k]);
            double diff = observed - expected_per_category;
            chi_squared += (diff * diff) / expected_per_category;
        }

        size_t df = accepted.size() - 1;
        double critical_value = get_chi_squared_critical_val_p001(df);

        EXPECT_LE(chi_squared, critical_value)
            << "Chi-Squared test failed (chi_squared=" << chi_squared
            << ", critical_value=" << critical_value << " for df=" << df
            << ", p <= 0.01). Distribution among accepted sentinels is not pseudo-random uniform!";
    }

    TEST_F(SentinelInjectionTest, AsymptoticContextOverridePriorityAndExclusionOver1000Seeds)
    {
        std::vector<SentinelKind> base_accepted = { SentinelKind::Assignment, SentinelKind::Return };
        std::vector<SentinelKind> base_excluded = { SentinelKind::Import };

        std::vector<SentinelKind> override_accepted = { SentinelKind::Reassignment, SentinelKind::ExprStmt };
        std::vector<SentinelKind> override_excluded = { SentinelKind::Function, SentinelKind::Directive };

        auto available_contexts = ContextRegistry::get_all_for(InjectableType::Expression);
        ASSERT_FALSE(available_contexts.empty());
        std::string_view target_context = available_contexts[0].name;

        ContextOverrideAny override_spec(
            target_context,
            std::nullopt,
            std::nullopt,
            override_excluded,
            override_accepted
        );

        ProcessingItem base_item{
            .type = InjectableType::Expression,
            .code = "1",
            .verifier = NullVerifier{},
            .path_name = "AsymptoticPriorityTest",
            .cumulative_prefix = "",
            .depth = 0,
            .recursion_depth = 0,
            .skip_contexts = {},
            .is_skipped = false,
            .context_overrides = { override_spec },
            .custom_errors = std::nullopt,
            .excluded_sentinels = base_excluded,
            .accepted_sentinels = base_accepted
        };

        bool override_path_found = false;

        std::map<SentinelKind, size_t> accumulated_counts;
        for (auto k : SentinelKinds::All) accumulated_counts[k] = 0;
        size_t total_generated_overall = 0;

        expand_to_top_level_stream(
            { base_item },
            [&](ProcessingItem&& item)
            {
                if (item.has_context(target_context))
                {
                    override_path_found = true;

                    for (auto k : base_excluded)
                    {
                        bool found = std::find(item.excluded_sentinels.begin(), item.excluded_sentinels.end(), k) != item.excluded_sentinels.end();
                        EXPECT_TRUE(found) << "Base excluded sentinel " << to_string(k) << " missing in overridden item!";
                    }
                    for (auto k : override_excluded)
                    {
                        bool found = std::find(item.excluded_sentinels.begin(), item.excluded_sentinels.end(), k) != item.excluded_sentinels.end();
                        EXPECT_TRUE(found) << "Override excluded sentinel " << to_string(k) << " missing in overridden item!";
                    }

                    for (auto k : item.accepted_sentinels)
                    {
                        bool is_override_accepted = std::find(override_accepted.begin(), override_accepted.end(), k) != override_accepted.end();
                        EXPECT_TRUE(is_override_accepted)
                            << "Accepted sentinel " << to_string(k) << " in item was not in override_accepted!";

                        bool is_base_accepted = std::find(base_accepted.begin(), base_accepted.end(), k) != base_accepted.end();
                        EXPECT_FALSE(is_base_accepted)
                            << "Base accepted sentinel " << to_string(k) << " appeared despite override priority!";
                    }

                    for (size_t seed = 1; seed <= 500; ++seed)
                    {
                        RecoveryBlock block = RecoverySentinel::generate_block_sentinel(
                            seed,
                            BlockContext::TopLevel,
                            item.excluded_sentinels,
                            item.accepted_sentinels
                        );

                        if (block.kind.has_value())
                        {
                            SentinelKind k = *block.kind;
                            total_generated_overall++;
                            accumulated_counts[k]++;
                        }
                    }
                }
            },
            false,
            std::nullopt
        );

        EXPECT_TRUE(override_path_found) << "Target context '" << target_context << "' was not exercised.";
        ASSERT_GT(total_generated_overall, 0);

        for (auto k : SentinelKinds::All)
        {
            bool is_in_override = std::find(override_accepted.begin(), override_accepted.end(), k) != override_accepted.end();
            if (!is_in_override)
            {
                EXPECT_EQ(accumulated_counts[k], 0)
                    << "Non-override sentinel kind " << to_string(k) << " appeared " << accumulated_counts[k]
                    << " times (expected EXACTLY 0)!";
            }
        }

        size_t sum_override_accepted = 0;
        for (auto k : override_accepted)
        {
            sum_override_accepted += accumulated_counts[k];
        }
        EXPECT_EQ(sum_override_accepted, total_generated_overall)
            << "100% of generated sentinels must belong to override_accepted!";

        double expected_per_kind = static_cast<double>(total_generated_overall) / static_cast<double>(override_accepted.size());
        double chi_squared = 0.0;
        for (auto k : override_accepted)
        {
            double observed = static_cast<double>(accumulated_counts[k]);
            double diff = observed - expected_per_kind;
            chi_squared += (diff * diff) / expected_per_kind;
        }

        size_t df = override_accepted.size() - 1;
        double critical_val = get_chi_squared_critical_val_p001(df);
        EXPECT_LE(chi_squared, critical_val)
            << "Chi-Squared test failed (chi_squared=" << chi_squared
            << ", critical_val=" << critical_val << " for df=" << df
            << "). Distribution across override_accepted sentinels is not 50%/50% uniform!";
    }

    TEST_F(SentinelInjectionTest, SentinelInBothAcceptedAndExcludedIsFilteredOut)
    {
        std::vector<SentinelKind> accepted = { SentinelKind::Assignment, SentinelKind::Reassignment };
        std::vector<SentinelKind> excluded = { SentinelKind::Assignment };

        for (size_t seed = 1; seed <= 500; ++seed)
        {
            RecoveryBlock block = RecoverySentinel::generate_block_sentinel(
                seed,
                BlockContext::TopLevel,
                excluded,
                accepted
            );

            if (block.kind.has_value())
            {
                EXPECT_NE(*block.kind, SentinelKind::Assignment)
                    << "SentinelKind::Assignment was in both accepted and excluded, but appeared in generated block!";
                EXPECT_EQ(*block.kind, SentinelKind::Reassignment);
            }
        }
    }

    TEST_F(SentinelInjectionTest, ExcludeAllSentinelsProducesEmptyRecoveryBlocks)
    {
        std::vector<SentinelKind> exclude_all = SentinelKinds::all();

        for (size_t seed = 1; seed <= 100; ++seed)
        {
            RecoveryBlock block = RecoverySentinel::generate_block_sentinel(
                seed,
                BlockContext::TopLevel,
                exclude_all,
                {}
            );

            EXPECT_FALSE(block.kind.has_value())
                << "When all sentinels are excluded, block.kind must be nullopt!";
            EXPECT_TRUE(block.source.empty())
                << "When all sentinels are excluded, block.source must be empty string!";
        }
    }

    TEST_F(SentinelInjectionTest, BaseAcceptedSentinelExcludedByOverrideIsFilteredOut)
    {
        std::vector<SentinelKind> base_accepted = { SentinelKind::Assignment };
        std::vector<SentinelKind> override_excluded = { SentinelKind::Assignment };

        auto available_contexts = ContextRegistry::get_all_for(InjectableType::Expression);
        ASSERT_FALSE(available_contexts.empty());
        std::string_view target_context = available_contexts[0].name;

        ContextOverrideAny override_spec(
            target_context,
            std::nullopt,
            std::nullopt,
            override_excluded,
            {}
        );

        ProcessingItem base_item{
            .type = InjectableType::Expression,
            .code = "1",
            .verifier = NullVerifier{},
            .path_name = "BaseAcceptedExcludedByOverrideTest",
            .cumulative_prefix = "",
            .depth = 0,
            .recursion_depth = 0,
            .skip_contexts = {},
            .is_skipped = false,
            .context_overrides = { override_spec },
            .custom_errors = std::nullopt,
            .excluded_sentinels = {},
            .accepted_sentinels = base_accepted
        };

        bool override_path_found = false;

        expand_to_top_level_stream(
            { base_item },
            [&](ProcessingItem&& item)
            {
                if (item.has_context(target_context))
                {
                    override_path_found = true;

                    bool assignment_accepted = std::find(item.accepted_sentinels.begin(),
                                                         item.accepted_sentinels.end(),
                                                         SentinelKind::Assignment) != item.accepted_sentinels.end();
                    EXPECT_FALSE(assignment_accepted)
                        << "Base accepted SentinelKind::Assignment was NOT filtered out despite being excluded by override!";

                    bool assignment_excluded = std::find(item.excluded_sentinels.begin(),
                                                         item.excluded_sentinels.end(),
                                                         SentinelKind::Assignment) != item.excluded_sentinels.end();
                    EXPECT_TRUE(assignment_excluded)
                        << "SentinelKind::Assignment missing from item.excluded_sentinels!";
                }
            },
            false,
            std::nullopt
        );

        EXPECT_TRUE(override_path_found) << "Target context '" << target_context << "' was not exercised.";
    }

    INSTANTIATE_TEST_SUITE_P(
        SentinelInjectionTests,
        SentinelInjectionTest,
        testing::ValuesIn(GetTestingFrameworkSamples()),
        TestNameGenerator{}
    );
}
