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
        static void DumpKeywordTest(const std::string& keyword,
                                    const std::vector<std::string_view>& skip_contexts = {},
                                    const std::vector<ContextOverrideAny>& context_overrides = {},
                                    const std::vector<SentinelKind>& excluded_sentinels = {},
                                    const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            DumpWriter writer("reserved_keyword_debug_" + keyword + ".txt", "reserved_keyword_dumps");
            if (!writer.is_open()) return;
            auto& out = writer.out();

            out << "============================================================\n";
            out << "RESERVED KEYWORD DUMP FOR: " << keyword << "\n";
            out << "============================================================\n\n";

            size_t count = 0;
            ParserExpectedError base_error(ParserErrorCode::ReservedKeywordAsIdentifier, 1, 1, 1, keyword.length() + 1);

            size_t scenario_index = 0;
            size_t base_seed = RecoverySentinel::make_seed("ReservedKeywordDebugger_" + keyword);

            expand_to_top_level_stream(
                InjectableType::Identifier,
                keyword,
                UniversalVerifier(keyword),
                "KeywordTest",
                [&](ProcessingItem&& item)
                {
                    if (item.is_skipped) return;

                    count++;
                    size_t seed = base_seed + (scenario_index++ * 2);

                    auto prog = BuildRecoveryProgram(item, seed);
                    auto shifted_errors = ErrorShifter::shift_errors(prog.prefix_for_shifting, {base_error});

                    out << "--- VARIATION " << count << " ---\n";
                    out << "PATH:  " << item.path_name << "\n";
                    out << "CODE:\n" << prog.full_code << "\n";
                    out << "EXPECTED ERRORS:\n";
                    for (const auto& err : shifted_errors)
                    {
                        out << "  - Code: " << static_cast<int>(std::get<ParserErrorCode>(err.code))
                            << ", Line: " << err.line_start
                            << ", Col: " << err.column_start << "\n";
                    }
                    out << "------------------------------------------------------------\n\n";
                },
                true,
                skip_contexts,
                context_overrides,
                std::nullopt,
                excluded_sentinels,
                accepted_sentinels
            );

            out << "[DEBUG] Reserved keyword dump finished (" << count << " variations)\n";
        }
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ReservedKeywordDebugger, GenerateKeywordReport)
    {
        DumpKeywordTest("struct");
        DumpKeywordTest("let");
        DumpKeywordTest("mod");
    }
#endif
}
