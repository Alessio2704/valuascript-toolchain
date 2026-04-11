#include <gtest/gtest.h>
#include "frontend/parser/parser_errors_synchronization_base.h"
#include "token/reserved_keyword_lookup.h"
#include "stages/frontend/parser/ast.h"
#include <algorithm>
#include "frontend/parser/shared_following_constructs.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test {
    namespace {
        auto all_following_constructs = get_all_top_level_following_constructs();

        std::vector<ParserErrorsSynchronizationTestCase> GenerateExhaustiveRhsTests() {
            std::vector<ParserErrorsSynchronizationTestCase> test_cases;
            auto all_keywords = get_all_reserved_keyword_strings();

            test_cases.reserve(all_keywords.size());
            for (const auto &keyword: all_keywords) {
                for (const auto &following_construct: all_following_constructs) {
                    std::string test_name = "rhs_keyword_" + keyword + "_recovers_" + following_construct.name;
                    std::string source_code = "typealias Broken = " + keyword + "\n" + following_construct.source;

                    test_cases.push_back({
                        test_name,
                        source_code,
                        {{Err::ReservedKeywordAsIdentifier, 1, 20}},
                        [verify_following_construct = following_construct.verify](const Program &program) {
                            auto it = std::find_if(program.type_aliases.begin(), program.type_aliases.end(),
                                                   [](const auto &type_alias) { return type_alias->name == "Broken"; });

                            if (it != program.type_aliases.end()) {
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

    class TypeAliasRhsResilienceTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TypeAliasRhsResilienceTest, RecoversAnyValidConstructAfterIllegalRhsKeyword) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAliasExhaustiveResilience,
        TypeAliasRhsResilienceTest,
        ::testing::ValuesIn(GenerateExhaustiveRhsTests()),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
