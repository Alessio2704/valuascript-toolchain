#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "frontend/parser/ast.h"

namespace valuascript::compiler::test
{
    namespace
    {
        auto ExpectRecoveredAssignment(const std::string& expected_target_name = "a")
        {
            return [expected_target_name](const Program& ast)
            {
                ASSERT_FALSE(
                    ast.execution_steps.empty(
                    )) << "Expected at least one execution step indicating successful recovery";
                auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                ASSERT_NE(assign, nullptr) << "Expected first step to be an Assignment";
                ASSERT_FALSE(assign->targets.empty()) << "Expected at least one assignment target";
                EXPECT_EQ(assign->targets[0].name, expected_target_name);
            };
        }
    }

    class TypeAliasParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(TypeAliasParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAliasStressTests,
        TypeAliasParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "missing_assignment_operator",
            "typealias MyType int\n"
            "let a = 1\n",
            { {Err::ExpectedAssignAfterTypeAliasName, 1, 18} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.type_aliases.empty());
            ExpectRecoveredAssignment("a")(ast);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
