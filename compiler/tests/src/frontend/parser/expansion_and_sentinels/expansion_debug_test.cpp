#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/context_names.h"
#include <fstream>

namespace valuascript::compiler::test
{
    class ExpansionDebugger : public ParserTestBase
    {
    public:
        template <typename Verifier = NullVerifier>
        static void DumpExpansion(InjectableType type,
                                  const std::string& snippet,
                                  const std::string& label,
                                  const Verifier& verifier = NullVerifier{},
                                  const std::vector<std::string_view>& skip_contexts = {},
                                  const std::vector<ContextOverrideAny>& context_overrides = {},
                                  const std::vector<SentinelKind>& excluded_sentinels = {},
                                  const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            DumpWriter writer("expansion_features_debug_" + label + ".txt", "expansion_dumps");
            if (!writer.is_open()) return;
            auto& out = writer.out();

            out << "============================================================\n";
            out << "EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            size_t count = 0;
            expand_to_top_level_stream(type, snippet, verifier, label, [&](ProcessingItem&& item)
            {
                count++;
                out << "--- VARIATION " << count << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "CODE:\n" << item.code << "\n";
                out << "------------------------------------------------------------\n\n";
            }, false, skip_contexts, context_overrides, std::nullopt, excluded_sentinels, accepted_sentinels);

            out << "[DEBUG] Recovery expansion dump finished (" << count << " variations)";
        }
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ExpansionDebugger, GenerateExpressionReport)
    {
        DumpExpansion(InjectableType::Expression, "1 + 1", "BasicArithmetic");
        DumpExpansion(InjectableType::TypeAnnotation, "int", "BasicType");
        DumpExpansion(InjectableType::Modifier, "@modifier", "BasicModifier");
        DumpExpansion(InjectableType::WeakStatement, "return 1", "BasicReturn");
        DumpExpansion(InjectableType::StrongStatement, "let res = 1", "BasicAssign");
    }
#endif
}
