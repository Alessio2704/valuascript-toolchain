#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "token/reserved_keyword_lookup.h"
#include "frontend/parser/helpers/recovery_sentinel.h"
#include "frontend/parser/helpers/error_shifter.h"

namespace valuascript::compiler::test
{
    class ReservedKeywordDebugger : public ParserTestBase
    {
    public:
        static void DumpKeywordTest(const std::string& keyword)
        {
            DumpWriter writer("reserved_keyword_debug_" + keyword + ".txt");
            if (!writer.is_open()) return;
            auto& out = writer.out();

            out << "============================================================\n";
            out << "RESERVED KEYWORD DUMP FOR: " << keyword << "\n";
            out << "============================================================\n\n";

            size_t count = 0;
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
                    count++;
                    size_t seed = std::hash<std::string>{}(item.path_name + keyword) ^ i;
                    auto pre_sentinel = RecoverySentinel::generate_top_level_sentinel(seed);
                    auto post_sentinel = post_sentinels[i];

                    std::string full_code = pre_sentinel.source + "\n\n" + item.code + "\n\n" + post_sentinel.source +
                        "\n";
                    std::string prefix_for_shifting = pre_sentinel.source + "\n\n" + item.cumulative_prefix;
                    auto shifted_errors = ErrorShifter::shift_errors(prefix_for_shifting, {base_error});

                    out << "--- VARIATION " << count << " ---\n";
                    out << "PATH:  " << item.path_name << "\n";
                    out << "CODE:\n" << full_code << "\n";
                    out << "EXPECTED ERRORS:\n";
                    for (const auto& err : shifted_errors)
                    {
                        out << "  - Code: " << static_cast<int>(std::get<ParserErrorCode>(err.code))
                            << ", Line: " << err.line_start
                            << ", Col: " << err.column_start << "\n";
                    }
                    out << "------------------------------------------------------------\n\n";
                }
            }, false, ExpansionPolicy{3, 1});

            out << "[DEBUG] Reserved keyword dump finished (" << count << " variations)\n";
        }
    };

    TEST_F(ReservedKeywordDebugger, GenerateKeywordReport)
    {
        DumpKeywordTest("struct");
    }
}
