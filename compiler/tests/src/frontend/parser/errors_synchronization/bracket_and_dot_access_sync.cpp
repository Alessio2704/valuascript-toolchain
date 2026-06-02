#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "frontend/parser/ast.h"

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

        void ExpectDotAccess(const Expression* expr, std::function<void(const Expression*)> target_verifier,
                             const std::string& property)
        {
            auto dot = dynamic_cast<const DotAccess*>(expr);
            ASSERT_NE(dot, nullptr) << "Expected DotAccess, but got " << (expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(dot->property_name, property);
            if (target_verifier) target_verifier(dot->target.get());
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
            "bracket_wrong_closing_preserves_index_and_next_stmt",
            "let a = arr[1}\n"
            "let b = 2\n",
            { {Err::UnmatchedBracketAfterTensorIndex, 1, 14} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            ExpectBracketAccess(assign_a->value.get(), [](auto target) { ExpectIdentifier(target, "arr"); }, [](auto i)
                {
                ExpectNumber(i, "1");
                });

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_slice_missing_right_bracket_preserves_slice",
            "let a = arr[1:2 \n"
            "let b = 2\n",
            { {Err::UnmatchedBracketAfterTensorIndex, 1, 16} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            ExpectBracketAccess(assign_a->value.get(), [](auto target) { ExpectIdentifier(target, "arr"); }, [](auto i)
                {
                ExpectSlice(i, [](auto l) { ExpectNumber(l, "1"); }, [](auto r) { ExpectNumber(r, "2"); });
                });
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_run_on_statement_eaten_by_expression_recovery",
            "let a = arr[\n"
            "let b = 2\n",
            {
            {Err::InvalidExpression, 1, 12},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].name, "a");
            ExpectBracketAccess(assign->value.get(), [](auto target) { ExpectIdentifier(target, "arr"); }, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_missing_property_discards_statement",
            "let a = obj.\n"
            "let b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].name, "a");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_slice_garbage_right_bound",
            "let a = arr[1 : *]\n",
            { {Err::InvalidExpression, 1, 17} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto target) { ExpectIdentifier(target, "arr"); }, [](auto i) {
                    ExpectNumber(i, "1");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_slice_garbage_left_bound",
            "let a = arr[* : 2]\n",
            { {Err::InvalidExpression, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto target) { ExpectIdentifier(target, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_missing_operator_between_elements",
            "let a = arr[1 2]\n",
            {
            {Err::MissingOperatorOrExpectedColonOrBracketInTensor, 1, 15}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto target) { ExpectIdentifier(target, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_aborts_assignment_preserves_next_statement",
            "let a = obj.\n"
            "let b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            ;
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);

            EXPECT_EQ(assign->targets[0].name, "a");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "postfix_deep_chain_with_multiple_internal_failures",
            "let a = obj.let[1, *].if[*]\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 13},
            {Err::UnexpectedCommaInBracketAccess, 1, 18},
            {Err::ReservedKeywordAsIdentifier, 1, 23},
            {Err::InvalidExpression, 1, 26}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto l3) {
                    ExpectDotAccess(l3, [](auto l2) {
                        ExpectBracketAccess(l2, [](auto l1) {
                            ExpectDotAccess(l1, [](auto obj) { ExpectIdentifier(obj, "obj"); }, "let");
                            }, [](auto idx) {
                            ASSERT_EQ(idx, nullptr);
                            });
                        }, "if");
                    }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_keyword_lookahead_chain",
            "let a = obj.let.if[0]\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 13},
            {Err::ReservedKeywordAsIdentifier, 1, 17}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto dot2) {
                    ExpectDotAccess(dot2, [](auto dot1) {
                        ExpectDotAccess(dot1, [](auto obj) { ExpectIdentifier(obj, "obj"); }, "let");
                        }, "if");
                    }, [](auto idx) { ExpectNumber(idx, "0"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_ident_assign",
            "let a = obj.\n"
            "b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[1].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectIdentifier(reassignment->target.get(), "b");
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_succeeds_at_newline_func_call_no_args",
            "let a = obj.\n"
            "test()\n",
            {},
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);

            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].name, "a");

            auto call = dynamic_cast<FunctionCall*>(assign->value.get());
            ASSERT_NE(call, nullptr);

            auto dot_access = dynamic_cast<DotAccess*>(call->target.get());
            ASSERT_NE(dot_access, nullptr);
            EXPECT_EQ(dot_access->property_name, "test");

            EXPECT_TRUE(call->arguments.empty());
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_succeeds_at_newline_func_call_with_args",
            "let a = obj.\n"
            "test(arg: 1)\n",
            {},
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);

            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].name, "a");

            auto call = dynamic_cast<FunctionCall*>(assign->value.get());
            ASSERT_NE(call, nullptr);

            auto dot_access = dynamic_cast<DotAccess*>(call->target.get());
            ASSERT_NE(dot_access, nullptr);
            ExpectIdentifier(dot_access->target.get(), "obj");
            EXPECT_EQ(dot_access->property_name, "test");

            ASSERT_EQ(call->arguments.size(), 1);
            EXPECT_EQ(call->arguments[0].first, "arg");
            ExpectNumber(call->arguments[0].second.get(), "1");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_dot_access_assign",
            "let a = obj.\n"
            "a.prop = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[1].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectDotAccess(reassignment->target.get(), [](auto target) { ExpectIdentifier(target, "a"); }, "prop");
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_bracket_access_assign",
            "let a = obj.\n"
            "a[0] = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[1].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectBracketAccess(reassignment->target.get(), [](auto target) { ExpectIdentifier(target, "a"); }, [](auto
                    i) {
                ExpectNumber(i, "0"); });
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_chained_dot_bracket_assign",
            "let a = obj.\n"
            "a.prop[0] = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[1].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectBracketAccess(reassignment->target.get(), [](auto target) {
                ExpectDotAccess(target, [](auto target2) { ExpectIdentifier(target2, "a"); }, "prop");
                }, [](auto i) { ExpectNumber(i, "0"); });
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_chained_bracket_dot_assign",
            "let a = obj.\n"
            "a[0].prop = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assignment = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assignment, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_access_unclosed_at_eof",
            "let a = arr[1",
            {
            {Err::UnmatchedBracketAfterTensorIndex, 1, 14},
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto target) { ExpectIdentifier(target, "arr"); }, [](auto i) {
                    ExpectNumber(i, "1");
                    });
                })
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
