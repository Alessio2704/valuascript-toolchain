#include <gtest/gtest.h>
#include "frontend/parser/helpers/deterministic_sampler.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/error_shifter.h"
#include "frontend/parser/helpers/context_names.h"

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
            std::vector<std::string_view> effective_skip_contexts = skip_contexts;
            if (keyword == "true" || keyword == "false" || keyword == "self" ||
                keyword == "if" || keyword == "switch" || keyword == "not" ||
                keyword == "else" || keyword == "then" || keyword == "case" || keyword == "default" || keyword == "return")
            {
                auto add_skip = [&](std::string_view name)
                {
                    if (std::find(effective_skip_contexts.begin(), effective_skip_contexts.end(), name) == effective_skip_contexts.end())
                    {
                        effective_skip_contexts.push_back(name);
                    }
                };
                add_skip(ContextNames::IdAsExpression);
            }

            DumpWriter writer("reserved_keyword_debug_" + keyword + ".txt", "reserved_keyword_dumps");
            if (!writer.is_open()) return;
            auto& out = writer.out();

            out << "============================================================\n";
            out << "RESERVED KEYWORD DUMP FOR: " << keyword << "\n";
            out << "============================================================\n\n";

            size_t count = 0;
            ParserExpectedError base_error(ParserErrorCode::ReservedKeywordAsIdentifier, 1, 1, 1, keyword.length() + 1);

            size_t scenario_index = 0;
            size_t base_seed = DeterministicSampler::make_seed("ReservedKeywordDebugger", keyword);

            expand_to_top_level_stream(
                InjectableType::Identifier,
                keyword,
                UniversalVerifier(keyword),
                "KeywordTest",
                [&](ProcessingItem&& item)
                {
                    if (item.is_skipped) return;

                    size_t seed = base_seed + (scenario_index++ * 2);

                    ForEachRecoveryProgram(item, seed, [&](const ConstructedRecoveryProgram& prog)
                    {
                        count++;
                        auto shifted_errors = ErrorShifter::shift_errors(prog.prefix_for_shifting, {base_error});

                        out << "--- VARIATION " << count << " ---\n";
                        out << "PATH:           " << (prog.path_name.empty() ? item.path_name : prog.path_name) << "\n";
                        print_sentinel_debug_info(out, prog.pre_kind, prog.is_pre_modified, prog.post_kind, prog.is_post_modified,
                                                  prog.inner_pre_kind, prog.is_inner_pre_modified, prog.inner_post_kind, prog.is_inner_post_modified);
                        out << "CODE:\n" << prog.full_code << "\n";
                        out << "EXPECTED ERRORS:\n";
                        for (const auto& err : shifted_errors)
                        {
                            out << "  - Code: " << static_cast<int>(std::get<ParserErrorCode>(err.code))
                                << ", Line: " << err.line_start
                                << ", Col: " << err.column_start << "\n";
                        }
                        out << "------------------------------------------------------------\n\n";
                    });
                },
                true,
                effective_skip_contexts,
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
