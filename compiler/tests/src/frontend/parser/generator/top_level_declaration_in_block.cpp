#include <gtest/gtest.h>
#include "../errors_synchronization/parser_errors_synchronization_base.h"
#include "language_constructs_provider.h"
#include "invalid_top_level_declaration_in_block.h"

namespace valuascript::compiler::test
{
    namespace
    {
        std::vector<ParserErrorsSynchronizationTestCase> GenerateTopLevelInBlockTests()
        {
            std::vector<ParserErrorsSynchronizationTestCase> test_cases;
            auto constructs = LanguageConstructsProvider::build_all_test_variants();
            auto envs = get_environments();

            for (const auto& env : envs)
            {
                for (const auto& construct : constructs)
                {
                    if (!construct.is_top_level_only) continue;

                    if (!env.supports_modifiers &&
                        construct.name.find("_mod") != std::string::npos &&
                        construct.name.find("_mod0") == std::string::npos)
                    {
                        continue;
                    }

                    std::string test_name = "illegal_top_level_" + construct.name + "_" + env.name;
                    std::string source = env.pre_code + construct.source + env.post_code;

                    auto [start_line, start_col] = get_end_position(env.pre_code);
                    auto span = calculate_expected_span(construct.source, start_line, start_col);

                    std::vector<ExpectedParserError> expected_errors;

                    if (env.expects_missing_value_error)
                    {
                        expected_errors.push_back({
                            Err::MissingValueAfterEquals,
                            span.line_start,
                            span.column_start,
                            span.line_start,
                            span.column_start + get_first_token_length(construct.source)
                        });
                    }

                    expected_errors.push_back({
                        Err::TopLevelDeclarationNotAllowedHere,
                        span.line_start,
                        span.column_start,
                        span.line_end,
                        span.column_end
                    });

                    test_cases.push_back({
                        test_name,
                        source,
                        expected_errors,
                        env.verify
                    });
                }
            }
            return test_cases;
        }
    }

    class TopLevelDeclarationInBlockTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(TopLevelDeclarationInBlockTest, RaisesErrorWithCorrectFullSpan)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(ParserScope,
                             TopLevelDeclarationInBlockTest,
                             ::testing::ValuesIn(GenerateTopLevelInBlockTests()),
                             [](const auto& test_info) { return test_info.param.test_name; });
}
