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

    std::vector<MissingOperatorDumpTestCase> missing_operator_dump_cases()
    {
        return {
            {
                .name = "MissingOperator1",
                .snippet = "1 2"
            },
            {
                .name = "MissingOperator2",
                .snippet = "1 a"
            },
            {
                .name = "MissingOperator3",
                .snippet = "a b"
            },
            {
                .name = "MissingOperator4",
                .snippet = "a 1"
            },
            {
                .name = "MissingOperator5",
                .snippet = "100 + 1 2"
            },
            {
                .name = "MissingOperator6",
                .snippet = "1 + (2 3)"
            },
            {
                .name = "MissingOperator7",
                .snippet = "1 (2 + 3)"
            },
            {
                .name = "MissingOperator8",
                .snippet = "1 + a() b()"
            },
            {
                .name = "MissingOperator9",
                .snippet = "1000 a() + b()"
            },
            {
                .name = "MissingOperator10",
                .snippet = "a + b (1 + 2)"
            },
            {
                .name = "MissingOperator11",
                .snippet = "a + b model.a"
            },
            {
                .name = "MissingOperator12",
                .snippet = "a + b vec[0]"
            },
            {
                .name = "MissingOperator13",
                .snippet = "a + b {}"
            },
            {
                .name = "MissingOperator14",
                .snippet = "a[1] + (b.a  c)"
            },
            {
                .name = "MissingOperator15",
                .snippet = "a[1] + (b.a  c[3].b)"
            },
            {
                .name = "MissingOperator16",
                .snippet = "a + a.key (1 + 2)"
            },
            {
                .name = "MissingOperator17",
                .snippet = "1 + a[0] + b[1:2] a.b"
            },
            {
                .name = "MissingOperator18",
                .snippet = "1 + a() (2 + 3)"
            },
            {
                .name = "MissingOperator19",
                .snippet = "a[1]  (b - c)"
            },
            {
                .name = "MissingOperator20",
                .snippet = "a[1] / (b  c)"
            },
            {
                .name = "MissingOperator21",
                .snippet = "a[1] / (1  c)"
            },
            {
                .name = "MissingOperator22",
                .snippet = "a[1] (1 + c)"
            },
            {
                .name = "MissingOperator23",
                .snippet = "a ([1, 2])"
            },
            {
                .name = "MissingOperator24",
                .snippet = "a ({1, 2})"
            },
            {
                .name = "MissingOperator25",
                .snippet = "a ([[1, 2], [3, 4]])"
            },
            {
                .name = "MissingOperator26",
                .snippet = "a (-5)"
            },
            {
                .name = "MissingOperator27",
                .snippet = "13_624 / 11%   4"
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
