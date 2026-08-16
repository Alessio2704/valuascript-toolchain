#include <gtest/gtest.h>
#include "frontend/parser/helpers/deterministic_sampler.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "invalid_declaration_in_block_constructs.h"

namespace valuascript::compiler::test
{
    class InvalidDeclarationInBlockTest : public ParserTestBase,
                                          public testing::WithParamInterface<InvalidDeclarationInBlockTestCase>
    {
    };

    TEST_P(InvalidDeclarationInBlockTest, VerifyErrorAndAstEquivalence)
    {
        const auto& test_case = GetParam();

        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = DeterministicSampler::make_seed(test_info ? test_info->name() : "fallback");

        for_each_invalid_declaration_program(test_case.context, test_case.construct_case, base_seed, [&](const ConstructedRecoveryProgram& prog)
        {
            SCOPED_TRACE("Testing Invalid Declaration in Block: " + test_case.test_name + "\n--- Full Source Code Listing ---\n" + format_source_with_lines(prog.full_code));

            CompilerContext context;
            context.settings.fail_fast = false;
            auto program = run_parser(prog.full_code, context);

            const auto& diagnostics = context.diagnostics.get_errors();

            SourceSpan expected_span = compute_expected_span(prog.full_code, test_case.construct_case.code, prog.prefix_for_shifting.length());

            const ValuaScriptException* target_diag = nullptr;
            for (const auto& err : diagnostics)
            {
                if (std::holds_alternative<ParserErrorCode>(err.get_code()) &&
                    std::get<ParserErrorCode>(err.get_code()) == ParserErrorCode::InvalidConstructPlacement)
                {
                    target_diag = &err;
                    break;
                }
            }

            ASSERT_NE(target_diag, nullptr)
                << "Expected ParserErrorCode::InvalidConstructPlacement in context " << test_case.context.name
                << " for source:\n" << format_source_with_lines(prog.full_code);

            const auto& span = target_diag->get_span();
            EXPECT_EQ(span.line_start, expected_span.line_start)
                << "Expected placement error line_start " << expected_span.line_start
                << " but found " << span.line_start
                << " in context " << test_case.context.name
                << " for source:\n" << format_source_with_lines(prog.full_code);

            if (test_case.construct_case.is_broken)
            {
                for (const auto& suppressed_code : test_case.construct_case.suppressed_errors)
                {
                    bool found_suppressed = false;
                    if (suppressed_code == ValuascriptErrorCode(ParserErrorCode::InvalidConstructPlacement))
                    {
                        size_t count = 0;
                        for (const auto& err : diagnostics)
                        {
                            if (err.get_code() == suppressed_code) count++;
                        }
                        if (count > 1) found_suppressed = true;
                    }
                    else
                    {
                        for (const auto& err : diagnostics)
                        {
                            if (err.get_code() == suppressed_code)
                            {
                                found_suppressed = true;
                                break;
                            }
                        }
                    }

                    EXPECT_FALSE(found_suppressed)
                        << "Internal error code " << std::visit(
                            [](auto&& c) { return std::to_string(static_cast<int>(c)); }, suppressed_code)
                        << " should have been suppressed for invalid declaration in block context:\n"
                        << format_source_with_lines(prog.full_code);
                }
            }

            ExpectProgram(program.get(), prog.full_spec);
        });
    }

    INSTANTIATE_TEST_SUITE_P(
        InvalidDeclarationInBlock,
        InvalidDeclarationInBlockTest,
        testing::ValuesIn(GenerateInvalidDeclarationInBlockTestCases()),
        TestNameGenerator{}
    );
}
