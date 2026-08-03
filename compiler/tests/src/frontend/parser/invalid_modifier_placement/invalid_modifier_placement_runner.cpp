#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "invalid_modifier_placement_shared.h"

namespace valuascript::compiler::test
{
    class InvalidModifierPlacementTest : public ParserTestBase,
                                         public testing::WithParamInterface<InvalidModifierPlacementTestCase>
    {
    };

    TEST_P(InvalidModifierPlacementTest, VerifyErrorAndAstEquivalence)
    {
        const auto& test_case = GetParam();
        SCOPED_TRACE("Testing Invalid Modifier Placement: " + test_case.test_name);

        std::string snippet = build_invalid_modifier_snippet(test_case);
        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = std::hash<std::string>{}(test_info ? test_info->name() : "fallback");
        size_t scenario_index = 0;

        expand_to_top_level_stream(
            test_case.construct_case.type,
            snippet,
            test_case.construct_case.verifier,
            test_case.test_name,
            [&](ProcessingItem&& processed)
            {
                ProgramSpec inner_spec;
                std::visit([&](auto&& ver) { SpecAdder::add(inner_spec, ver); }, processed.verifier);

                auto prog = BuildRecoveryProgram(processed, inner_spec, base_seed + (scenario_index++ * 2));

                CompilerContext context;
                context.settings.fail_fast = false;
                auto program = run_parser(prog.full_code, context);

                bool has_modifier_error = false;
                for (const auto& err : context.diagnostics.get_errors())
                {
                    if (std::holds_alternative<ParserErrorCode>(err.get_code()) &&
                        std::get<ParserErrorCode>(err.get_code()) == ParserErrorCode::ModifiersAttachedToInvalidDeclaration)
                    {
                        has_modifier_error = true;
                        break;
                    }
                }

                EXPECT_TRUE(has_modifier_error)
                    << "Expected ModifiersAttachedToInvalidDeclaration error in expanded code:\n"
                    << format_source_with_lines(prog.full_code);

                ExpectProgram(program.get(), prog.full_spec);
            },
            true,
            test_case.construct_case.skip_contexts
        );
    }

    INSTANTIATE_TEST_SUITE_P(
        InvalidModifierPlacement,
        InvalidModifierPlacementTest,
        testing::ValuesIn(GenerateInvalidModifierTestCases()),
        [](const testing::TestParamInfo<InvalidModifierPlacementTestCase>& param_info) {
            return param_info.param.test_name;
        }
    );
}
