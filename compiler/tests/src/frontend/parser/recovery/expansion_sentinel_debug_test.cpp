#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <iostream>

namespace valuascript::compiler::test
{
    class ExpansionRecoveryDebugger : public ParserTestBase
    {
    public:
        static void DumpRecoveryExpansion(InjectableType type,
                                          const std::string& snippet,
                                          const std::vector<ExpectedError>& errors,
                                          const std::string& label,
                                          const UniversalVerifier& verifier)
        {
            size_t debug_seed = std::hash<std::string>{}(label);

            auto scenarios = generate_recovery_scenarios(type, snippet, errors, verifier, label, debug_seed);

            DumpWriter writer("expansion_sentinel_recovery_debug_" + label + ".txt");
            if (!writer.is_open()) return;

            auto& out = writer.out();

            out << "============================================================\n";
            out << "RECOVERY EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "Total variations (Sentinels + Expansion): " << scenarios.size() << "\n";
            out << "============================================================\n\n";

            for (size_t i = 0; i < scenarios.size(); ++i)
            {
                const auto& sc = scenarios[i];
                out << "--- VARIATION " << i + 1 << " ---\n";
                out << "PATH:  " << sc.path_name << "\n";
                out << "DEPTH: " << sc.depth << "\n";
                out << "EXPECTED SHIFTED ERRORS:\n";
                for (const auto& err : sc.shifted_errors)
                {
                    out << "  - Code: " << static_cast<int>(err.code)
                        << " at [" << err.line_start << ":" << err.column_start << "]\n";
                }
                out << "FULL CODE:\n";
                out << sc.full_code;
                out << "------------------------------------------------------------\n\n";
            }

            std::cout << "[DEBUG] Recovery expansion dump finished: " << writer.path_string() << "\n";
        }
    };

    TEST_F(ExpansionRecoveryDebugger, GenerateRecoveryReport)
    {
        DumpRecoveryExpansion(
            InjectableType::StrongStatement,
            "let x = ",
            {ExpectedError(ValuascriptErrorCode::MissingValueAfterEquals, 1, 7)},
            "BrokenAssignment",
            AssignmentVerifier([](Assignment*)
            {
            })
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1 + ",
            {ExpectedError(ValuascriptErrorCode::InvalidExpression, 1, 5)},
            "MalformedBinary",
            ExprVerifier([](Expression*)
            {
            })
        );
    }
}
