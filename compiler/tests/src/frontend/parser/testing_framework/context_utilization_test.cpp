#include "testing_framework_helpers.h"
#include <algorithm>
#include <vector>
#include <string>

#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/expansion_and_sentinels/expansion_calculator.h"

namespace valuascript::compiler::test
{
    class ContextUtilizationTest : public TestingFrameworkTestBase,
                                   public testing::WithParamInterface<TestingFrameworkSample>
    {
    };

    TEST_P(ContextUtilizationTest, UseAllContextsWhenNoSkipProvided)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        std::vector<ProcessingItem> results;
        expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
        {
            results.push_back(std::move(item));
        }, false);

        ASSERT_FALSE(results.empty()) << "Expansion produced zero items for " << test_name;

        size_t aug_count = get_augmentation_count(start_type, snippet, NullVerifier{}, test_name, {});
        size_t expected_count = ExpansionCalculator::compute_expected_expansions(start_type, {}) * aug_count;
        EXPECT_EQ(results.size(), expected_count)
            << "Expansion count mismatch for " << test_name << " when no skip_contexts are provided.";

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        for (const auto& ctx : available_contexts)
        {
            bool context_used = std::any_of(results.begin(), results.end(), [&](const auto& item) {
                return has_context_segment(item, ctx.name);
            });
            EXPECT_TRUE(context_used) << "Context '" << ctx.name
                                      << "' is valid for " << test_name << " but was never exercised.";
        }
    }

    TEST_P(ContextUtilizationTest, SkipContextsExcludedFromExpansion)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        if (available_contexts.empty()) return;

        for (const auto& target_ctx : available_contexts)
        {
            std::vector<std::string_view> skip_contexts = { target_ctx.name };

            std::vector<ProcessingItem> results;
            expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
            {
                results.push_back(std::move(item));
            }, false, skip_contexts);

            size_t aug_count = get_augmentation_count(start_type, snippet, NullVerifier{}, test_name, skip_contexts);
            size_t expected_count = ExpansionCalculator::compute_expected_expansions(start_type, skip_contexts) * aug_count;
            EXPECT_EQ(results.size(), expected_count)
                << "Expansion count mismatch when skipping context '" << target_ctx.name << "'.";

            for (const auto& item : results)
            {
                EXPECT_FALSE(has_context_segment(item, target_ctx.name))
                    << "Skipped context '" << target_ctx.name << "' appeared in path: " << item.path_name;
            }
        }
    }

    TEST_P(ContextUtilizationTest, ContextOverrideIsApplied)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        if (available_contexts.empty()) return;

        for (const auto& target_ctx : available_contexts)
        {
            std::string_view target_context_name = target_ctx.name;

            ContextOverrideAny override_item(target_context_name,
                                             std::nullopt,
                                             std::nullopt,
                                             { SentinelKind::Import, SentinelKind::Directive });

            std::vector<ProcessingItem> results;
            expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
            {
                results.push_back(std::move(item));
            }, false, {}, { override_item });

            ASSERT_FALSE(results.empty());

            bool override_verified = false;
            for (const auto& item : results)
            {
                if (has_context_segment(item, target_context_name))
                {
                    override_verified = true;
                    bool has_import_excl = std::find(item.excluded_sentinels.begin(),
                                                     item.excluded_sentinels.end(),
                                                     SentinelKind::Import) != item.excluded_sentinels.end();
                    bool has_directive_excl = std::find(item.excluded_sentinels.begin(),
                                                        item.excluded_sentinels.end(),
                                                        SentinelKind::Directive) != item.excluded_sentinels.end();
                    EXPECT_TRUE(has_import_excl && has_directive_excl)
                        << "Context override excluded_sentinels were not propagated for context '" << target_context_name << "'";
                }
            }
            EXPECT_TRUE(override_verified) << "Target override context '" << target_context_name << "' was not exercised in any path.";
        }
    }

    TEST_P(ContextUtilizationTest, ContextOverrideDefaultFalseAllowsRecursion)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        if (available_contexts.empty()) return;

        for (const auto& target_ctx : available_contexts)
        {
            std::string_view target_context_name = target_ctx.name;

            ContextOverrideAny override_item(target_context_name,
                                             std::nullopt,
                                             std::nullopt,
                                             {},
                                             {},
                                             /* skip_after_d0 = */ false);

            std::vector<ProcessingItem> results;
            expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
            {
                results.push_back(std::move(item));
            }, false, {}, { override_item });

            size_t aug_count = get_augmentation_count(start_type, snippet, NullVerifier{}, test_name, {});
            size_t expected_count = ExpansionCalculator::compute_expected_expansions(start_type, {}) * aug_count;
            EXPECT_EQ(results.size(), expected_count)
                << "Expansion count mismatch for skip_after_depth_0 = false on context '" << target_context_name << "'";

            bool context_used = std::any_of(results.begin(), results.end(), [&](const auto& item) {
                return has_context_segment(item, target_context_name);
            });
            EXPECT_TRUE(context_used) << "Context '" << target_context_name
                                      << "' was not used when skip_after_depth_0 = false";
        }
    }

    TEST_P(ContextUtilizationTest, ContextOverrideSkipAfterDepth0BehavesLikeSkipContexts)
    {
        const auto& [start_type, snippet, test_name] = GetParam();

        auto available_contexts = ContextRegistry::get_all_for(start_type);
        if (available_contexts.empty()) return;

        for (const auto& target_ctx : available_contexts)
        {
            std::string_view target_context_name = target_ctx.name;

            ContextOverrideAny override_item(target_context_name,
                                             std::nullopt,
                                             std::nullopt,
                                             {},
                                             {},
                                             /* skip_after_d0 = */ true);

            std::vector<ProcessingItem> override_results;
            expand_to_top_level_stream(start_type, snippet, NullVerifier{}, test_name, [&](ProcessingItem&& item)
            {
                override_results.push_back(std::move(item));
            }, false, {}, { override_item });

            bool used_at_depth_0 = false;
            bool used_at_depth_gt_0 = false;
            for (const auto& item : override_results)
            {
                for (size_t i = 0; i < item.context_history.size(); ++i)
                {
                    if (item.context_history[i].context_name == target_context_name)
                    {
                        if (i == 0) used_at_depth_0 = true;
                        else used_at_depth_gt_0 = true;
                    }
                }
            }
            EXPECT_TRUE(used_at_depth_0) << "Target override context '" << target_context_name << "' was not exercised at depth 0.";
            EXPECT_FALSE(used_at_depth_gt_0) << "Target override context '" << target_context_name << "' appeared at depth > 0 even though skip_after_depth_0 = true.";
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ContextUtilizationTests,
        ContextUtilizationTest,
        testing::ValuesIn(GetTestingFrameworkSamples()),
        TestNameGenerator{}
    );
}
