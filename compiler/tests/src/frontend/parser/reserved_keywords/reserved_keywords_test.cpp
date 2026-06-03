#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "token/reserved_keyword_lookup.h"
#include "frontend/parser/helpers/recovery_sentinel.h"
#include "frontend/parser/helpers/error_shifter.h"

namespace valuascript::compiler::test
{
    class ReservedKeywordRecoveryTest : public ParserTestBase,
                                        public testing::WithParamInterface<std::string>
    {
    };

    TEST_P(ReservedKeywordRecoveryTest, KeywordRejectedAndRecoveredExhaustively)
    {
        std::string keyword = GetParam();

        ParserExpectedError base_error(ParserErrorCode::ReservedKeywordAsIdentifier, 1, 1, 1, keyword.length() + 1);

        auto items = apply_context_augmentations(
            InjectableType::Identifier,
            keyword,
            UniversalVerifier(keyword),
            "KeywordTest"
        );

        expand_to_top_level_stream(std::move(items), [&](ProcessingItem&& item)
        {
            if (item.is_skipped) return;

            const auto& post_sentinels = RecoverySentinel::get_all_top_level_sentinels();

            for (size_t i = 0; i < post_sentinels.size(); ++i)
            {
                size_t seed = std::hash<std::string>{}(item.path_name + keyword) ^ i;
                auto pre_sentinel = RecoverySentinel::generate_top_level_sentinel(seed);

                const auto& post_sentinel = post_sentinels[i];

                std::string full_code = pre_sentinel.source + "\n\n" + item.code + "\n\n" + post_sentinel.source + "\n";
                std::string prefix_for_shifting = pre_sentinel.source + "\n\n" + item.cumulative_prefix;

                ProgramSpec full_spec;
                if (pre_sentinel.add_to_spec) pre_sentinel.add_to_spec(full_spec);

                std::visit([&](auto&& ver) { SpecAdder::add(full_spec, ver); }, item.verifier);

                if (post_sentinel.add_to_spec) post_sentinel.add_to_spec(full_spec);

                auto shifted_errors = ErrorShifter::shift_errors(prefix_for_shifting, {base_error});

                SCOPED_TRACE("Context Path: " + item.path_name);

                ExpectParseErrors(full_code, shifted_errors, full_spec);
            }
        }, true, ExpansionPolicy{3, 1});
    }

    INSTANTIATE_TEST_SUITE_P(
        ExhaustiveKeywords,
        ReservedKeywordRecoveryTest,
        testing::ValuesIn(shared::get_all_reserved_keyword_strings()),
        [](const testing::TestParamInfo<std::string>& test_info) {
        return test_info.param;
        }
    );
}
