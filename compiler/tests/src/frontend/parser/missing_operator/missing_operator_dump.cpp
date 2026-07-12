#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <iostream>
#include "missing_operator_shared.h"

namespace valuascript::compiler::test
{
    class MissingOperatorDump : public ParserTestBase,
                                public testing::WithParamInterface<MissingOperatorTemplateBase>
    {
    protected:
        void run_dump_for_snippet(const std::string& name, const std::string& snippet)
        {
            auto items = apply_context_augmentations(InjectableType::Expression, snippet, NullVerifier{}, name);

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

            expand_to_top_level_stream(std::move(items), [&](ProcessingItem&& out_item)
            {
                if (out_item.is_skipped) return;

                auto prog = BuildRecoveryProgram(std::move(out_item.code), ProgramSpec{},
                                                 out_item.cumulative_prefix,
                                                 base_seed + (file_index * 2));

                out << "--- VARIATION " << (file_index + 1) << " ---\n";
                out << "PATH:  " << out_item.path_name << "\n";
                out << "DEPTH: " << out_item.depth << "\n";
                out << "FULL CODE:\n";
                out << prog.full_code;
                out << "------------------------------------------------------------\n\n";

                file_index++;
            }, true);

            out << "[DEBUG] Recovery expansion dump finished (" << file_index << " variations)\n";
        }
    };

    TEST_P(MissingOperatorDump, InspectGeneratedPrograms)
    {
        auto tc = GetParam();
        const auto& atoms = get_atoms();

        std::cout << "\n======================================================\n";
        std::cout << "Dumping missing operator recovery programs to: missing_operator_dumps/\n";
        std::cout << "Template: " << tc.test_name << "\n";
        std::cout << "======================================================\n\n";

        if (tc.type == TemplateType::TwoLeaves)
        {
            for (const auto& a : atoms)
            {
                for (const auto& b : atoms)
                {
                    std::string snippet = a.code + " " + b.code;
                    run_dump_for_snippet(tc.test_name + "_" + a.name + "_" + b.name, snippet);
                }
            }
        }
        else if (tc.type == TemplateType::ThreeLeaves)
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
        else if (tc.type == TemplateType::SpecialCases)
        {
            for (const auto& special_case : get_special_cases())
            {
                run_dump_for_snippet(tc.test_name + "_" + special_case.test_name, special_case.snippet);
            }
        }
    }

    std::vector<MissingOperatorTemplateBase> missing_operator_dump_templates()
    {
        return {
            {"TwoLeaves", TemplateType::TwoLeaves},
            {"ThreeLeaves", TemplateType::ThreeLeaves},
            {"FourLeaves", TemplateType::FourLeaves},
            {"SpecialCases", TemplateType::SpecialCases}
        };
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        MissingOperatorDump,
        testing::ValuesIn(missing_operator_dump_templates()),
        [](const testing::TestParamInfo<MissingOperatorTemplateBase>& param_info) {
        return param_info.param.test_name;
        }
    );
}
