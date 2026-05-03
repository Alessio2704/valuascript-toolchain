#include <gtest/gtest.h>
#include "../errors_synchronization/parser_errors_synchronization_base.h"
#include "token/reserved_keyword_lookup.h"
#include "frontend/parser/ast.h"
#include <algorithm>
#include "language_constructs_provider.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test
{
    namespace
    {
        std::vector<ParserErrorsSynchronizationTestCase> GenerateResilienceTests()
        {
            std::vector<ParserErrorsSynchronizationTestCase> test_cases;
            auto keywords = get_all_reserved_keyword_strings();
            auto followers = LanguageConstructsProvider::build_all_test_variants();

            for (const auto& kw : keywords)
            {
                for (const auto& follow : followers)
                {
                    std::string source = "typealias Broken = " + kw + "\n" + follow.source;

                    test_cases.push_back({
                        "alias_resilience_" + kw + "_to_" + follow.name,
                        source,
                        {{Err::ReservedKeywordAsIdentifier, 1, 20}},
                        [verify_following_construct = follow.verify](const Program& program)
                        {
                            auto it = std::find_if(program.type_aliases.begin(), program.type_aliases.end(),
                                                   [](const auto& type_alias) { return type_alias->name == "Broken"; });

                            if (it != program.type_aliases.end())
                            {
                                EXPECT_EQ((*it)->target_type, nullptr)
                                    << "Broken alias was kept but contains an invalid target_type.";
                            }

                            verify_following_construct(program);
                        }
                    });
                }
            }
            return test_cases;
        }
    }

    class TypeAliasResilienceTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(TypeAliasResilienceTest, VerifyRecovery)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserResilience,
        TypeAliasResilienceTest,
        ::testing::ValuesIn(GenerateResilienceTests()),
        [](const auto& test_info)
        {
        return test_info.param.test_name;
        }
    );
}
