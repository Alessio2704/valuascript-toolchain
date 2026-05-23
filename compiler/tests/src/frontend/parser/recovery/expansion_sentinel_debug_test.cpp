#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/error_shifter.h"

namespace valuascript::compiler::test
{
    class ExpansionRecoveryDebugger : public ParserTestBase
    {
    public:
        static void DumpRecoveryExpansion(InjectableType type,
                                          const std::string& snippet,
                                          const std::vector<ParserExpectedError>& errors,
                                          const std::string& label,
                                          const UniversalVerifier& verifier)
        {
            DumpWriter writer("expansion_sentinel_recovery_debug_" + label + ".txt");
            if (!writer.is_open()) return;

            auto& out = writer.out();

            out << "============================================================\n";
            out << "RECOVERY EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            size_t scenario_index = 0;
            size_t base_seed = std::hash<std::string>{}(label);

            expand_to_top_level_stream({type, snippet, verifier, label, "", 0, 0}, [&](ProcessingItem&& item)
            {
                ProgramSpec item_spec;
                std::visit([&](auto&& ver) { SpecAdder::add(item_spec, ver); }, item.verifier);

                auto prog = BuildRecoveryProgram(
                    std::move(item.code),
                    std::move(item_spec),
                    std::move(item.cumulative_prefix),
                    base_seed + (scenario_index * 2)
                );

                scenario_index++;

                out << "--- VARIATION " << scenario_index << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "EXPECTED SHIFTED ERRORS (Sample calculation):\n";

                auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, errors);
                for (const auto& err : shifted)
                {
                    out << "  - Code: " << err.code
                        << " at [" << err.line_start << ":" << err.column_start << "]\n";
                }

                out << "FULL CODE:\n";
                out << prog.full_code;
                out << "------------------------------------------------------------\n\n";
            }, true);

            out << "[DEBUG] Recovery expansion dump finished (" << scenario_index
                << " variations)";
        }
    };

    TEST_F(ExpansionRecoveryDebugger, GenerateRecoveryReport)
    {
        DumpRecoveryExpansion(
            InjectableType::StrongStatement,
            "let x = ",
            {ParserExpectedError(ParserErrorCode::MissingValueAfterEquals, 1, 7)},
            "BrokenAssignment",
            AssignmentVerifier([](Assignment*)
            {
            })
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1 + ",
            {ParserExpectedError(ParserErrorCode::InvalidExpression, 1, 5)},
            "MalformedBinary",
            ExprVerifier([](Expression*)
            {
            })
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "map<string, *, int>",
            {ParserExpectedError(ParserErrorCode::MissingTypeAnnotation, 1, 13)},
            "BrokenTypeAnnotation",
            TypeVerifier([](TypeAnnotation*)
            {
            })
        );

        DumpRecoveryExpansion(
            InjectableType::Modifier,
            "@test(a 1, b: 2)",
            {ParserExpectedError(ParserErrorCode::MissingColonAfterArgument, 1, 9)},
            "BrokenModifier",
            std::vector<ModifierSpec>{}
        );
    }
}
