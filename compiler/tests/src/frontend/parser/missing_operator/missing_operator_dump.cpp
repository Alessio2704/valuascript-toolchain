#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <iostream>
#include "missing_operator_shared.h"

namespace valuascript::compiler::test
{
    class MissingOperatorDumpBase : public ParserTestBase
    {
    protected:
        template <typename Verifier = NullVerifier>
        static void run_dump_for_snippet(const std::string& name,
                                        const std::string& snippet,
                                        const Verifier& verifier = NullVerifier{},
                                        const std::vector<std::string_view>& skip_contexts = {},
                                        const std::vector<ContextOverrideAny>& context_overrides = {},
                                        const std::vector<SentinelKind>& excluded_sentinels = {},
                                        const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            size_t file_index = 0;
            size_t base_seed = 0;

            std::string safe_snippet = snippet;
            std::replace(safe_snippet.begin(), safe_snippet.end(), '/', '_');
            std::string filename = safe_snippet + ".txt";
            DumpWriter writer(filename, "missing_operator_dumps");
            ASSERT_TRUE(writer.is_open()) << "Failed to open file for writing: " << writer.path_string();

            auto& out = writer.out();
            out << "============================================================\n";
            out << "RECOVERY EXPANSION DUMP FOR: " << name << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            expand_to_top_level_stream(InjectableType::Expression, snippet, verifier, name, [&](ProcessingItem&& out_item)
            {
                if (out_item.is_skipped) return;

                ForEachRecoveryProgram(out_item, ProgramSpec{}, base_seed + (file_index * 2), [&](const ConstructedRecoveryProgram& prog)
                {
                    out << "--- VARIATION " << (file_index + 1) << " ---\n";
                    out << "PATH:  " << (prog.path_name.empty() ? out_item.path_name : prog.path_name) << "\n";
                    out << "DEPTH: " << out_item.depth << "\n";
                    out << "FULL CODE:\n";
                    out << prog.full_code;
                    out << "------------------------------------------------------------\n\n";

                    file_index++;
                });
            }, true, skip_contexts, context_overrides, std::nullopt, excluded_sentinels, accepted_sentinels);

            out << "[DEBUG] Recovery expansion dump finished (" << file_index << " variations)\n";
        }
    };

    class MissingOperatorTwoLeavesDump : public MissingOperatorDumpBase,
                                         public testing::WithParamInterface<TwoLeavesPairDef>
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_P(MissingOperatorTwoLeavesDump, InspectGeneratedPrograms)
    {
        auto tc = GetParam();
        std::string snippet = tc.a.code + " " + tc.b.code;
        run_dump_for_snippet("TwoLeaves_" + tc.a.name + "_" + tc.b.name, snippet);
    }

    INSTANTIATE_TEST_SUITE_P(
        TwoLeaves,
        MissingOperatorTwoLeavesDump,
        testing::ValuesIn(get_two_leaves_pairs()),
        TestNameGenerator{}
    );

    class MissingOperatorSpecialCasesDump : public MissingOperatorDumpBase,
                                            public testing::WithParamInterface<SpecialCaseDef>
    {
    };

    TEST_P(MissingOperatorSpecialCasesDump, InspectGeneratedPrograms)
    {
        const auto& special_case = GetParam();
        run_dump_for_snippet("SpecialCases_" + special_case.test_name, special_case.snippet);
    }

    INSTANTIATE_TEST_SUITE_P(
        SpecialCases,
        MissingOperatorSpecialCasesDump,
        testing::ValuesIn(get_special_cases()),
        TestNameGenerator{}
    );

    class MissingOperatorDump : public MissingOperatorDumpBase,
                                public testing::WithParamInterface<MissingOperatorTemplateBase>
    {
    };

    TEST_P(MissingOperatorDump, InspectGeneratedPrograms)
    {
        auto tc = GetParam();
        const auto& atoms = get_atoms();

        if (tc.type == TemplateType::ThreeLeaves)
        {
            {
                const auto& a = atoms[0];
                const auto& b = atoms[1];
                const auto& c = atoms[2];

                {
                    std::string snippet = a.code + " " + b.code + " + " + c.code;
                    run_dump_for_snippet(tc.test_name + "_Pos1_" + a.name + "_" + b.name + "_" + c.name, snippet);
                }
                {
                    std::string snippet = a.code + " + " + b.code + " " + c.code;
                    run_dump_for_snippet(tc.test_name + "_Pos2_" + a.name + "_" + b.name + "_" + c.name, snippet);
                }
            }
        }
        else if (tc.type == TemplateType::FourLeaves)
        {
            {
                const auto& a = atoms[0];
                const auto& b = atoms[1];
                const auto& c = atoms[2];
                const auto& d = atoms[3];

                {
                    std::string snippet = a.code + " " + b.code + " + " + c.code + " + " + d.code;
                    run_dump_for_snippet(
                        tc.test_name + "_Pos1_" + a.name + "_" + b.name + "_" + c.name + "_" + d.name, snippet);
                }
                {
                    std::string snippet = a.code + " + " + b.code + " " + c.code + " + " + d.code;
                    run_dump_for_snippet(
                        tc.test_name + "_Pos2_" + a.name + "_" + b.name + "_" + c.name + "_" + d.name, snippet);
                }
                {
                    std::string snippet = a.code + " + " + b.code + " + " + c.code + " " + d.code;
                    run_dump_for_snippet(
                        tc.test_name + "_Pos3_" + a.name + "_" + b.name + "_" + c.name + "_" + d.name, snippet);
                }
            }
        }
    }

    std::vector<MissingOperatorTemplateBase> missing_operator_dump_templates()
    {
        return {
            {.test_name="ThreeLeaves", .type=TemplateType::ThreeLeaves},
            {.test_name="FourLeaves", .type=TemplateType::FourLeaves}
        };
    }

    INSTANTIATE_TEST_SUITE_P(
        ExpressionTemplates,
        MissingOperatorDump,
        testing::ValuesIn(missing_operator_dump_templates()),
        TestNameGenerator{}
    );
#endif
}
