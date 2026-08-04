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
                .test_name = "missing_assignment_operator",
                .source_code = "typealias MyType int\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedAssignAfterTypeAliasName, .line = 1, .column = 18} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_TRUE(ast.type_aliases.empty());
                    ExpectRecoveredAssignment("a")(ast);
                }
            }
        ),
        TestNameGenerator{}
    );
}
