#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "token/reserved_keyword_lookup.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test {
    namespace {
        auto ExpectDiscardedAliasAndRecoveredAssignment() {
            return [](const Program &ast) {
                EXPECT_TRUE(ast.type_aliases.empty()) << "Alias node should have been discarded.";
                ASSERT_FALSE(ast.execution_steps.empty()) << "Parser failed to recover next statement.";
                auto assign = dynamic_cast<Assignment *>(ast.execution_steps[0].get());
                ASSERT_NE(assign, nullptr);
                EXPECT_EQ(assign->targets[0].first, "recovery_var");
            };
        }

        std::vector<ParserErrorsSynchronizationTestCase> GenerateRhsKeywordTests() {
            std::vector<ParserErrorsSynchronizationTestCase> cases;
            auto all_keywords = get_all_reserved_keyword_strings();

            cases.reserve(all_keywords.size());
            for (const auto &keyword: all_keywords) {
                cases.push_back({
                    "rhs_illegal_keyword_" + keyword,
                    "typealias Test = " + keyword + "\nlet recovery_var = 1\n",
                    {{Err::ReservedKeywordAsIdentifier, 1, 18}},
                    ExpectDiscardedAliasAndRecoveredAssignment()
                });
            }
            return cases;
        }
    }

    class TypeAliasRhsKeywordGeneratorTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TypeAliasRhsKeywordGeneratorTest, UnifiedErrorAndRecoveryForEveryReservedKeyword) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAliasExhaustiveTests,
        TypeAliasRhsKeywordGeneratorTest,
        ::testing::ValuesIn(GenerateRhsKeywordTests()),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
