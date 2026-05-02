#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <fstream>

namespace valuascript::compiler::test
{
    class ExpansionDebugger : public ParserTestBase
    {
    public:
        static void DumpExpansion(InjectableType type, const std::string& snippet,
                                  const std::string& label, const UniversalVerifier& verifier)
        {
            DumpWriter writer("expansion_features_debug_" + label + ".txt");
            if (!writer.is_open()) return;
            auto& out = writer.out();

            out << "============================================================\n";
            out << "EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            size_t count = 0;
            expand_to_top_level_stream({type, snippet, verifier, label, "", 0, 0}, [&](ProcessingItem&& item)
            {
                count++;
                out << "--- VARIATION " << count << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "CODE:\n" << item.code << "\n";
                out << "------------------------------------------------------------\n\n";
            }, false);

            out << "[DEBUG] Recovery expansion dump finished (" << count
                << " variations)";
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
