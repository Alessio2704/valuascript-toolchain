#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <fstream>
#include <iostream>

namespace valuascript::compiler::test
{
    class ExpansionDebugger : public ParserTestBase
    {
    public:
        static void DumpExpansion(InjectableType type, const std::string& snippet, const std::string& label,
                                  const UniversalVerifier& verifier)
        {
            auto results = expand_to_top_level(type, snippet, verifier, label);

            DumpWriter writer("expansion_features_debug_" + label + ".txt");
            if (!writer.is_open()) return;

            auto& out = writer.out();

            out << "============================================================\n";
            out << "EXPANSION DUMP FOR: " << label << " (" << snippet << ")\n";
            out << "Total variations generated: " << results.size() << "\n";
            out << "============================================================\n\n";

            for (size_t i = 0; i < results.size(); ++i)
            {
                const auto& item = results[i];
                out << "--- VARIATION " << i + 1 << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "TYPE:  " << static_cast<int>(item.type) << "\n";
                out << "CODE:\n";
                out << item.code << "\n";
                out << "------------------------------------------------------------\n\n";
            }

            std::cout << "\n[DEBUG] Expansion logic finished. Dumped " << results.size()
                << " variations to: " << writer.path_string() << "\n\n";
        }
    };

    TEST_F(ExpansionDebugger, GenerateExpressionReport)
    {
        DumpExpansion(InjectableType::Expression, "1 + 1", "BasicArithmetic", ExprVerifier([](Expression*)
        {
        }));

        DumpExpansion(InjectableType::TypeAnnotation, "int", "BasicType", TypeVerifier([](TypeAnnotation*)
        {
        }));

        DumpExpansion(InjectableType::Modifier, "@modifier", "BasicModifier", ModifierVerifier
                      {
                      });

        DumpExpansion(InjectableType::WeakStatement, "return 1", "BasicReturn", ReturnVerifier([](ReturnStatement*)
        {
        }));

        DumpExpansion(InjectableType::StrongStatement, "let res = 1", "BasicAssign", AssignmentVerifier([](Assignment*)
        {
        }));
    }
}
