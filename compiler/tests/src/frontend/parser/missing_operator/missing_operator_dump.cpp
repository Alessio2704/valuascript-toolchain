#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <iostream>

namespace valuascript::compiler::test
{
    struct MissingOperatorDumpTestCase
    {
        std::string name;
        std::string snippet;
    };

    class MissingOperatorDump : public ParserTestBase,
                                public testing::WithParamInterface<MissingOperatorDumpTestCase>
    {
    };

    TEST_P(MissingOperatorDump, InspectGeneratedPrograms)
    {
        const auto& [name, snippet] = GetParam();

        std::cout << "\n======================================================\n";
        std::cout << "Dumping missing operator recovery programs to: missing_operator_dumps/\n";
        std::cout << "======================================================\n\n";

        auto items = apply_context_augmentations(InjectableType::Expression, snippet, NullVerifier{}, name);

        size_t file_index = 0;
        size_t base_seed = 0;

        std::string filename = "missing_operator_all_expansions_" + name + ".vs";
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

            auto prog = BuildRecoveryProgram(std::move(out_item.code), ProgramSpec{}, out_item.cumulative_prefix,
                                             base_seed + (file_index * 2));

            out << "--- VARIATION " << (file_index + 1) << " ---\n";
            out << "PATH:  " << out_item.path_name << "\n";
            out << "DEPTH: " << out_item.depth << "\n";
            out << "FULL CODE:\n";
            out << prog.full_code;
            out << "------------------------------------------------------------\n\n";

            file_index++;
        }, true);

        out << "[DEBUG] Recovery expansion dump finished (" << file_index << " variations)";
    }

    std::vector<MissingOperatorDumpTestCase> missing_operator_dump_cases() {
        return {
            {
                .name = "MissingOperator1",
                .snippet = "1 2"
            }
        };
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        MissingOperatorDump,
        testing::ValuesIn(missing_operator_dump_cases()),
        [](const testing::TestParamInfo<MissingOperatorDumpTestCase>& param_info) {
            return param_info.param.name;
        }
    );
}
