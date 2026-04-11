#include <gtest/gtest.h>
#include "../parser_errors_synchronization_base.h"
#include "token/reserved_keyword_lookup.h"
#include "stages/frontend/parser/ast.h"
#include <algorithm>
#include "frontend/parser/shared_following_constructs.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test {
    namespace {
        auto following_constructs = get_all_top_level_following_constructs();

        std::vector<ParserErrorsSynchronizationTestCase> GenerateExhaustiveRhsTests() {
            std::vector<ParserErrorsSynchronizationTestCase> cases;
            auto all_keywords = get_all_reserved_keyword_strings();

            cases.reserve(all_keywords.size());
            for (const auto &keyword: all_keywords) {
                for (const auto &fc: following_constructs) {
                    std::string test_name = "rhs_keyword_" + keyword + "_recovers_" + fc.name;
                    std::string source = "typealias Broken = " + keyword + "\n" + fc.source;

                    cases.push_back({
                        test_name,
                        source,
                        {{Err::ReservedKeywordAsIdentifier, 1, 20}},
                        [fc_verify = fc.verify](const Program &ast) {
                            auto it = std::find_if(ast.type_aliases.begin(), ast.type_aliases.end(),
                                                   [](const auto &t) { return t->name == "Broken"; });
                            EXPECT_EQ(it, ast.type_aliases.end()) << "Broken alias should have been discarded.";

                            fc_verify(ast);
                        }
                    });
                }
            }
            return cases;
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
