#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler::test {
    namespace {
        void ExpectIdentifier(const Expression *expr, const std::string &name) {
            auto id = dynamic_cast<const IdentifierAccess *>(expr);
            ASSERT_NE(id, nullptr) << "Expected IdentifierAccess, but got " << (
                                      expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(id->name, name);
        }

        void ExpectNumber(const Expression *expr, const std::string &value) {
            auto num = dynamic_cast<const NumberLiteral *>(expr);
            ASSERT_NE(num, nullptr) << "Expected NumberLiteral, but got " << (expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(num->value, value);
        }

        void ExpectDotAccess(const Expression *expr, std::function<void(const Expression *)> target_verifier,
                             const std::string &property) {
            auto dot = dynamic_cast<const DotAccess *>(expr);
            ASSERT_NE(dot, nullptr) << "Expected DotAccess, but got " << (expr ? typeid(*expr).name() : "nullptr");
            EXPECT_EQ(dot->property_name, property);
            if (target_verifier) target_verifier(dot->target.get());
        }

        void ExpectBracketAccess(const Expression *expr, std::function<void(const Expression *)> target_verifier,
                                 std::function<void(const Expression *)> index_verifier) {
            auto bracket = dynamic_cast<const BracketAccess *>(expr);
            ASSERT_NE(bracket, nullptr) << "Expected BracketAccess, but got " << (expr
                                               ? typeid(*expr).name()
                                               : "nullptr");
            if (target_verifier) target_verifier(bracket->target.get());

            if (index_verifier) {
                index_verifier(bracket->index.get());
            } else {
                EXPECT_EQ(bracket->index, nullptr) << "Expected index to be nullptr (partial AST fallback)";
            }
        }

        void ExpectSlice(const Expression *expr, std::function<void(const Expression *)> left_verifier,
                         std::function<void(const Expression *)> right_verifier) {
            auto bin = dynamic_cast<const BinaryExpression *>(expr);
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

        auto VerifyAssignmentValue(std::function<void(const Expression *)> value_verifier, size_t expected_steps = 1,
                                   size_t step_index = 0) {
            return [=](const Program &ast) {
                ASSERT_EQ(ast.execution_steps.size(), expected_steps) << "Execution step count mismatch";
                auto assign = dynamic_cast<Assignment *>(ast.execution_steps[step_index].get());
                ASSERT_NE(assign, nullptr) << "Expected step " << step_index << " to be an Assignment";
                value_verifier(assign->value.get());
            };
        }
    }

    class BracketAndDotAccessParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(BracketAndDotAccessParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        BracketAndDotAccessStressTests,
        BracketAndDotAccessParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "bracket_empty",
            "let a = arr[]\n",
            { {Err::EmptyBracketAccess, 1, 12} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_unexpected_comma",
            "let a = arr[1, 2]\n",
            { {Err::UnexpectedCommaInBracketAccess, 1, 14} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_invalid_expression_inside",
            "let a = arr[*]\n",
            { {Err::InvalidExpression, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_wrong_closing_preserves_index_and_next_stmt",
            "let a = arr[1}\n"
            "let b = 2\n",
            { {Err::UnmatchedBracketAfterTensorIndex, 1, 14} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign_a = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_a, nullptr);
            ExpectBracketAccess(assign_a->value.get(), [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                ExpectNumber(i, "1");
                });

            auto assign_b = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
            ASSERT_NE(assign_b, nullptr);
            EXPECT_EQ(assign_b->targets[0].first, "b");
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
            ExpectBracketAccess(assign_a->value.get(), [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                ExpectSlice(i, [](auto l) { ExpectNumber(l, "1"); }, [](auto r) { ExpectNumber(r, "2"); });
                });
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_run_on_statement_eaten_by_expression_recovery",
            "let a = arr[\n"
            "let b = 2\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 2, 1},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "a");
            ExpectBracketAccess(assign->value.get(), [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_recovers_to_next_chained_access",
            "let a = arr[*].prop\n",
            { {Err::InvalidExpression, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                ExpectDotAccess(expr, [](auto t) {
                    ExpectBracketAccess(t, [](auto arr) { ExpectIdentifier(arr, "arr"); }, nullptr);
                    }, "prop");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_missing_property_discards_statement",
            "let a = obj.\n"
            "let b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_garbage_property_discards_statement",
            "let a = obj.*\n"
            "let b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_garbage_fails_entire_chain",
            "let a = obj.*[1]\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            EXPECT_TRUE(ast.execution_steps.empty()) <<
            "Expected the entire statement to be dropped during synchronization";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_slice_garbage_right_bound",
            "let a = arr[1 : *]\n",
            { {Err::InvalidExpression, 1, 17} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                    ExpectNumber(i, "1");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_slice_garbage_left_bound",
            "let a = arr[* : 2]\n",
            { {Err::InvalidExpression, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_nested_failure_preserves_outer",
            "let a = arr[nested[*]]\n",
            { {Err::InvalidExpression, 1, 20} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                    ExpectBracketAccess(i, [](auto inner_t) { ExpectIdentifier(inner_t, "nested"); }, nullptr);
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_missing_operator_between_elements",
            "let a = arr[1 2]\n",
            {
            {Err::MissingOperatorOrExpectedColonOrBracketInTensor, 1, 15}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_chaining_middle_failure",
            "let a = arr[1][*][2]\n",
            { {Err::InvalidExpression, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto b2) {
                    ExpectBracketAccess(b2, [](auto b1) {
                        ExpectBracketAccess(b1, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) { ExpectNumber(i,
                            "1"); });
                        }, nullptr);
                    }, [](auto i) { ExpectNumber(i, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_reserved_keyword_property",
            "let a = obj.let\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                ExpectDotAccess(expr, [](auto t) { ExpectIdentifier(t, "obj"); }, "let");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_chained_abort",
            "let a = obj.prop1.*.prop3\n",
            { {Err::ExpectedPropertyName, 1, 19} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_aborts_assignment_preserves_next_statement",
            "let a = obj.\n"
            "let b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1) << "The broken statement should be completely dropped from the AST"
            ;
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);

            EXPECT_EQ(assign->targets[0].first, "b");
            ExpectNumber(assign->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_multiple_colons_yields_partial_slice",
            "let a = arr[1:2:3]\n",
            { {Err::UnmatchedBracketAfterTensorIndex, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                    ExpectSlice(i, [](auto l) { ExpectNumber(l, "1"); }, [](auto r) { ExpectNumber(r, "2"); });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_slice_missing_right_side_with_immediate_close_is_valid",
            "let a = arr[1:]\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                    ExpectSlice(i, [](auto l) { ExpectNumber(l, "1"); }, nullptr);
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_recovers_and_resumes_complex_chain_on_broken_target",
            "let a = arr[*].prop[2]\n",
            { {Err::InvalidExpression, 1, 13} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto dot_tgt) {
                    ExpectDotAccess(dot_tgt, [](auto brk_tgt) {
                        ExpectBracketAccess(brk_tgt, [](auto arr) { ExpectIdentifier(arr, "arr"); }, nullptr);
                        }, "prop");
                    }, [](auto idx) { ExpectNumber(idx, "2"); });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_consecutive_invalid_accesses",
            "let a = arr[*][*]\n",
            {
            {Err::InvalidExpression, 1, 13},
            {Err::InvalidExpression, 1, 16}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto inner_brk) {
                    ExpectBracketAccess(inner_brk, [](auto t) { ExpectIdentifier(t, "arr"); }, nullptr);
                    }, nullptr);
                })
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_tensor_literal_index_partial_ast",
            "let a = arr[ [1, *] ]\n",
            { {Err::InvalidExpression, 1, 18} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                    auto tensor = dynamic_cast<const TensorLiteral*>(i);
                    ASSERT_NE(tensor, nullptr) << "Expected inner expression to be a TensorLiteral";
                    ASSERT_EQ(tensor->elements.size(), 1);
                    ExpectNumber(tensor->elements[0].get(), "1");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_successive_reserved_keywords_as_properties",
            "let a = obj.let.if.else\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 13},
            {Err::ReservedKeywordAsIdentifier, 1, 17},
            {Err::ReservedKeywordAsIdentifier, 1, 20}
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectDotAccess(expr, [](auto t_else) {
                    ExpectDotAccess(t_else, [](auto t_if) {
                        ExpectDotAccess(t_if, [](auto t_let) { ExpectIdentifier(t_let, "obj"); }, "let");
                        }, "if");
                    }, "else");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_double_operator_aborts_chain_and_drops_statement",
            "let a = obj..prop\n"
            "let b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_on_failed_grouping_target",
            "let a = (obj + *).prop\n",
            { {Err::InvalidExpression, 1, 16} },
            VerifyAssignmentValue([](auto expr) {
                ExpectDotAccess(expr, [](auto tgt) {
                    auto group = dynamic_cast<const GroupingExpression*>(tgt);
                    ASSERT_NE(group, nullptr) << "Target should be a GroupingExpression";
                    EXPECT_EQ(group->expression.get(), nullptr);
                    }, "prop");
                })
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
            "bracket_nested_slice_recovery",
            "let a = data[1 : arr[*]]\n",
            { {Err::InvalidExpression, 1, 22} },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto data) { ExpectIdentifier(data, "data"); }, [](auto slice) {
                    ExpectSlice(slice, [](auto l) { ExpectNumber(l, "1"); }, [](auto r) {
                        ExpectBracketAccess(r, [](auto arr) { ExpectIdentifier(arr, "arr"); }, nullptr);
                        });
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_ident_assign",
            "let a = obj.\n"
            "b = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectIdentifier(reassignment->target.get(), "b");
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_func_call_no_args",
            "let a = obj.\n"
            "test()\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto expr_stmt = dynamic_cast<ExpressionStatement*>(ast.execution_steps[0].get());
            ASSERT_NE(expr_stmt, nullptr);
            auto call = dynamic_cast<FunctionCall*>(expr_stmt->expr.get());
            ASSERT_NE(call, nullptr);
            ExpectIdentifier(call->target.get(), "test");
            EXPECT_TRUE(call->arguments.empty());
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_func_call_with_args",
            "let a = obj.\n"
            "test(arg: 1)\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto expr_stmt = dynamic_cast<ExpressionStatement*>(ast.execution_steps[0].get());
            ASSERT_NE(expr_stmt, nullptr);
            auto call = dynamic_cast<FunctionCall*>(expr_stmt->expr.get());
            ASSERT_NE(call, nullptr);
            ExpectIdentifier(call->target.get(), "test");
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
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectDotAccess(reassignment->target.get(), [](auto t) { ExpectIdentifier(t, "a"); }, "prop");
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_fails_at_newline_bracket_access_assign",
            "let a = obj.\n"
            "a[0] = 2\n",
            { {Err::ExpectedPropertyName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectBracketAccess(reassignment->target.get(), [](auto t) { ExpectIdentifier(t, "a"); }, [](auto i) {
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
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectBracketAccess(reassignment->target.get(), [](auto t) {
                ExpectDotAccess(t, [](auto t2) { ExpectIdentifier(t2, "a"); }, "prop");
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
            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto reassignment = dynamic_cast<Reassignment*>(ast.execution_steps[0].get());
            ASSERT_NE(reassignment, nullptr);
            ExpectDotAccess(reassignment->target.get(), [](auto t) {
                ExpectBracketAccess(t, [](auto t2) { ExpectIdentifier(t2, "a"); }, [](auto i) { ExpectNumber(i, "0"); })
                ;
                }, "prop");
            ExpectNumber(reassignment->value.get(), "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "bracket_access_unclosed_at_eof",
            "let a = arr[1",
            {
            {Err::UnmatchedBracketAfterTensorIndex, 1, 14},
            },
            VerifyAssignmentValue([](auto expr) {
                ExpectBracketAccess(expr, [](auto t) { ExpectIdentifier(t, "arr"); }, [](auto i) {
                    ExpectNumber(i, "1");
                    });
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_on_complex_binary_failure",
            "let a = (a + *).prop\n",
            { {Err::InvalidExpression, 1, 14} },
            VerifyAssignmentValue([](auto expr) {
                ExpectDotAccess(expr, [](auto tgt) {
                    auto group = dynamic_cast<const GroupingExpression*>(tgt);
                    ASSERT_NE(group, nullptr);
                    EXPECT_EQ(group->expression, nullptr);
                    }, "prop");
                })
            },
            ParserErrorsSynchronizationTestCase{
            "dot_access_valid_multiline_chaining",
            "let a = obj\n"
            "  .prop1\n"
            "  .prop2\n",
            {},
            VerifyAssignmentValue([](auto expr) {
                ExpectDotAccess(expr, [](auto t1) {
                    ExpectDotAccess(t1, [](auto obj) { ExpectIdentifier(obj, "obj"); }, "prop1");
                    }, "prop2");
                })
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
