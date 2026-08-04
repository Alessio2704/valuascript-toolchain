#include <gtest/gtest.h>
#include "utils/parametrised_test_name_helper.h"
#include "frontend/parser/helpers/deterministic_sampler.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/context_registry.h"
#include "invalid_declaration_in_expression_constructs.h"

namespace valuascript::compiler::test
{
    struct InvalidDeclarationInExpressionContextDebugParam
    {
        std::string_view name;
        Context context;
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    class InvalidDeclarationInExpressionDebugger : public ParserTestBase,
                                                   public testing::WithParamInterface<InvalidDeclarationInExpressionContextDebugParam>
    {
    };

    TEST_P(InvalidDeclarationInExpressionDebugger, GenerateContextReport)
    {
        const auto& ctx = GetParam().context;
        SCOPED_TRACE("Debugging Invalid Declaration in Expression Context: " + std::string(ctx.name));

        DumpWriter writer("invalid_declaration_in_expression_debug_" + std::string(ctx.name) + ".txt",
                          "invalid_declaration_in_expression_dumps");
        ASSERT_TRUE(writer.is_open()) << "Failed to open dump file: " << writer.path_string();

        auto& out = writer.out();
        out << "============================================================\n";
        out << "RECOVERY EXPANSION DUMP FOR EXPR CONTEXT: " << ctx.name << "\n";
        out << "============================================================\n\n";

        const auto constructs = InvalidDeclarationInExpressionConstructRegistry::cases_for_context(ctx);
        size_t variation_index = 0;

        for (const auto& construct : constructs)
        {
            if (should_test_construct_in_context_expr(ctx, construct))
            {
                variation_index++;
                size_t base_seed = DeterministicSampler::make_seed(ctx.name, construct.name);

                auto prog = build_invalid_declaration_in_expression_program(ctx, construct, base_seed);

                out << "--- VARIATION " << variation_index << " (" << construct.name << ") ---\n";
                out << "TYPE:           " << (construct.is_broken ? "Broken" : "Clean") << "\n";

                if (construct.is_broken && !construct.suppressed_errors.empty())
                {
                    out << "SUPPRESSED:     ";
                    for (size_t i = 0; i < construct.suppressed_errors.size(); ++i)
                    {
                        if (i > 0) out << ", ";
                        out << std::visit([](auto&& c) { return std::to_string(static_cast<int>(c)); },
                                          construct.suppressed_errors[i]);
                    }
                    out << "\n";
                }

                out << "\nCONSTRUCT CODE:\n";
                out << construct.code;
                if (!construct.code.empty() && construct.code.back() != '\n') out << "\n";

                out << "\nFULL RECOVERY CODE:\n";
                out << prog.full_code;
                if (!prog.full_code.empty() && prog.full_code.back() != '\n') out << "\n";

                out << "------------------------------------------------------------\n\n";
            }
        }

        out << "[DEBUG] Recovery expansion dump finished (" << variation_index << " variations)\n";
    }

    inline std::vector<InvalidDeclarationInExpressionContextDebugParam> GetExpressionContextDebugParams()
    {
        std::vector<InvalidDeclarationInExpressionContextDebugParam> params;
        for (const auto& ctx : get_expression_test_contexts())
        {
            params.push_back({ .name = ctx.name, .context = ctx });
        }
        return params;
    }

    INSTANTIATE_TEST_SUITE_P(
        InvalidDeclarationInExpressionDebug,
        InvalidDeclarationInExpressionDebugger,
        testing::ValuesIn(GetExpressionContextDebugParams()),
        TestNameGenerator{}
    );
#endif
}
