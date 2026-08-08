#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        void ExpectIdentifier(const Expression* expr, const std::string& name)
        {
            auto id = dynamic_cast<const IdentifierAccess*>(expr);
            ASSERT_NE(id, nullptr) << "Expected IdentifierAccess, but got " << (
                expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(id->name, name);
        }

        void ExpectNumber(const Expression* expr, const std::string& value)
        {
            auto num = dynamic_cast<const NumberLiteral*>(expr);
            ASSERT_NE(num, nullptr) << "Expected NumberLiteral, but got " << (expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(num->value, value);
        }

        void ExpectBracketAccess(const Expression* expr, std::function<void(const Expression*)> target_verifier,
                                 std::function<void(const Expression*)> index_verifier)
        {
            auto bracket = dynamic_cast<const BracketAccess*>(expr);
            ASSERT_NE(bracket, nullptr) << "Expected BracketAccess, but got " << (expr
                    ? typeid(*expr).name()
                    : "nullptr");
            if (target_verifier) target_verifier(bracket->target.get());

            if (index_verifier)
            {
                index_verifier(bracket->index.get());
            }
            else
            {
                EXPECT_EQ(bracket->index, nullptr) << "Expected index to be nullptr (partial AST fallback)";
            }
        }

        void ExpectSlice(const Expression* expr, std::function<void(const Expression*)> left_verifier,
                         std::function<void(const Expression*)> right_verifier)
        {
            auto bin = dynamic_cast<const BinaryExpression*>(expr);
            ASSERT_NE(bin, nullptr) << "Expected BinaryExpression (slice), but got " << (
                expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(bin->op, TokenType::Colon);

            if (left_verifier) left_verifier(bin->left.get());
            else
                EXPECT_EQ(bin->left, nullptr);

            if (right_verifier) right_verifier(bin->right.get());
            else
                EXPECT_EQ(bin->right, nullptr);
        }

        auto VerifyAssignmentValue(std::function<void(const Expression*)> value_verifier, size_t expected_steps = 1,
                                   size_t step_index = 0)
        {
            return [=](const Program& ast)
            {
                ASSERT_EQ(ast.execution_steps.size(), expected_steps) << "Execution step count mismatch";
                auto assign = dynamic_cast<Assignment*>(ast.execution_steps[step_index].get());
                ASSERT_NE(assign, nullptr) << "Expected step " << step_index << " to be an Assignment";
                value_verifier(assign->value.get());
            };
        }
    }

    class BracketAndDotAccessParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(BracketAndDotAccessParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        BracketAndDotAccessStressTests,
        BracketAndDotAccessParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "bracket_slice_missing_right_bracket_preserves_slice",
                .source_code = "let a = arr[1:2 \nlet b = 2\n",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorIndex, .line = 1, .column = 15} },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(assign_a, nullptr);
                    ExpectBracketAccess(assign_a->value.get(), [](auto target) { ExpectIdentifier(target, "arr"); }, [](auto i) {
                        ExpectSlice(i, [](auto l) { ExpectNumber(l, "1"); }, [](auto r) { ExpectNumber(r, "2"); });
                    });
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "bracket_run_on_statement_eaten_by_expression_recovery",
                .source_code = "let a = arr[\nlet b = 2\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 1, .column = 12}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(assign, nullptr);
                    EXPECT_EQ(assign->targets[0].name, "a");
                    ExpectBracketAccess(assign->value.get(), [](auto target) { ExpectIdentifier(target, "arr"); }, nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "bracket_slice_garbage_right_bound",
                .source_code = "let a = arr[1 : *]\n",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 1, .column = 17} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectBracketAccess(expr, [](auto target) { ExpectIdentifier(target, "arr"); }, [](auto i) {
                        ExpectNumber(i, "1");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "bracket_slice_garbage_left_bound",
                .source_code = "let a = arr[* : 2]\n",
                .expected_errors = { {.code = Err::InvalidExpression, .line = 1, .column = 13} },
                .verify_ast = VerifyAssignmentValue([](auto expr) {
                    ExpectBracketAccess(expr, [](auto target) { ExpectIdentifier(target, "arr"); }, nullptr);
                })
            }
        ),
        TestNameGenerator{}
    );
}
