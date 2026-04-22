#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        struct ExpectedDictPair {
            std::string key;
            std::optional<std::string> expected_number_value;

            ExpectedDictPair(const char *k) : key(k), expected_number_value(std::nullopt) {
            }

            ExpectedDictPair(const char *k, const char *v) : key(k), expected_number_value(std::string(v)) {
            }

            ExpectedDictPair(const char *k, const std::optional<std::string> &v) : key(k), expected_number_value(v) {
            }
        };

        void ExpectDictLiteral(const Program &ast, const std::vector<ExpectedDictPair> &expected) {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto *dict = dynamic_cast<DictLiteral *>(assign->value.get());
            ASSERT_NE(dict, nullptr) << "Assignment value is not a DictLiteral.";

            ASSERT_EQ(dict->elements.size(), expected.size()) << "Dictionary pair count mismatch!";

            for (size_t i = 0; i < expected.size(); ++i) {
                EXPECT_EQ(dict->elements[i].key, expected[i].key) << "Dict key mismatch at index " << i;

                if (expected[i].expected_number_value.has_value()) {
                    auto *num = dynamic_cast<NumberLiteral *>(dict->elements[i].value.get());
                    ASSERT_NE(num, nullptr) << "Expected number literal for key: " << expected[i].key;
                    EXPECT_EQ(num->value, *expected[i].expected_number_value)
                        << "Value mismatch for key: " << expected[i].key;
                } else {
                    EXPECT_EQ(dict->elements[i].value, nullptr);
                }
            }
        }

        auto ExpectDict(std::vector<ExpectedDictPair> pairs) {
            return [pairs = std::move(pairs)](const Program &ast) {
                ExpectDictLiteral(ast, pairs);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

    class DictLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(DictLiteralParserSynchronizationTest, SynchronizesDictLiteralErrors) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        DictLiteralStressTests,
        DictLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "dict_missing_closing_brace",
            "let a = { x: 1 \n"
            "let recovery = 1\n",
            { {Err::UnmatchedBraceInDictionaryLiteral, 1, 15} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_multiple_closing_brace",
            "let a = { x: 1 \n"
            "let b = { x: 1 \n"
            "let c = { x: 1 \n"
            "let recovery = 1\n",
            {
            {Err::UnmatchedBraceInDictionaryLiteral, 1, 15},
            {Err::UnmatchedBraceInDictionaryLiteral, 2, 15},
            {Err::UnmatchedBraceInDictionaryLiteral, 3, 15}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 4);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_colon",
            "let a = { x 1, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedColonAfterDictionaryKey, 1, 13} },
            ExpectDict({ {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_comma",
            "let a = { x: 1 y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedCommaSeparatorInDictionaryLiteral, 1, 16} },
            ExpectDict({ {"x", "1"}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_key",
            "let a = { : 1, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 11} },
            ExpectDict({{"<error>", "1"}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_expression_value",
            "let a = { x: , y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 14} },
            [](const Program& ast) {
            auto assing_1 = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict_literal = dynamic_cast<DictLiteral*>(assing_1->value.get());
            EXPECT_NE(dict_literal, nullptr);
            EXPECT_EQ(dict_literal->elements.size(), 2);
            EXPECT_EQ(dict_literal->elements[0].value, nullptr);
            EXPECT_NE(dict_literal->elements[1].value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_is_reserved_keyword",
            "let a = { let: 1, func: 2 }\n"
            "let recovery = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 11},
            {Err::ReservedKeywordAsIdentifier, 1, 19}
            },
            ExpectDict({ {"let", "1"}, {"func", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_garbage_between_pairs",
            "let a = { x: 1, +, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 17} },
            ExpectDict({ {"x", "1"}, {"<error>", std::nullopt}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_double_comma",
            "let a = { x: 1,, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 16} },
            ExpectDict({ {"x", "1"}, {"<error>", std::nullopt}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_trailing_comma_allowed",
            "let a = { x: 1, }\n"
            "let recovery = 1\n",
            {},
            ExpectDict({ {"x", "1"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_is_literal_string",
            "let a = { \"key\": 1 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 11} },
            ExpectDict({{"<error>", "1"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_value_is_broken_expression",
            "let a = { x: 1 + *, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 18} },
            ExpectDict({{"x", std::nullopt}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "array_inside_dict_with_error",
            "let a = {a: [1, 2}\n"
            "let recovery = 1\n",
            { {Err::UnmatchedBracketAfterTensorElements, 1, 18} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 1);
            auto tensor = dynamic_cast<TensorLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(tensor, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_inside_array_with_error",
            "let a =[ { x: 1 + * }, { y: 2 } ]\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 19} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto* tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            ASSERT_NE(tensor, nullptr);
            ASSERT_EQ(tensor->elements.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_total_mangle",
            "let a = { x: 1, let y = 2\n"
            "let recovery = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 17},
            {Err::UnmatchedBraceInDictionaryLiteral, 1, 26}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_total_mangle_new_line",
            "let a = { x: 1,\n"
            "let y = 2\n"
            "let recovery = 1\n",
            { {Err::UnmatchedBraceInDictionaryLiteral, 1, 16} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 3);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_nested_brace_recovery",
            "let a = { x: { nested: 1 + * }, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 28} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto inner_dict = dynamic_cast<DictLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(inner_dict, nullptr);
            ASSERT_EQ(inner_dict->elements.size(), 1);
            EXPECT_EQ(inner_dict->elements[0].key, "nested");

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_multiple_errors_recovers_last",
            "let a = { x: *, y: +, z: 3 }\n"
            "let recovery = 1\n",
            {
            {Err::InvalidExpression, 1, 14},
            {Err::InvalidExpression, 1, 21}
            },
            ExpectDict({{"x", std::nullopt}, {"y", std::nullopt}, {"z", "3"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_eof_after_key",
            "let a = { x\n",
            {
            {Err::ExpectedColonAfterDictionaryKey, 1, 12},
            {Err::UnmatchedBraceInDictionaryLiteral, 1, 12}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            EXPECT_EQ(dict->elements.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_eof_after_colon",
            "let a = { x: \n",
            {
            {Err::InvalidExpression, 1, 13},
            {Err::UnmatchedBraceInDictionaryLiteral, 1, 13}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1);
            ExpectDictLiteral(ast, {{"x", std::nullopt}});
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_eof_after_comma",
            "let a = { x: 1, \n",
            {
            {Err::UnmatchedBraceInDictionaryLiteral, 1, 16}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            EXPECT_EQ(dict->elements.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_comma_after_complex_expression",
            "let a = { x: 1 + 2 y: 3 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedCommaSeparatorInDictionaryLiteral, 1, 20} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assignment = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assignment->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);
            ASSERT_EQ(dict->elements[0].key, "x");
            auto binary_expr = dynamic_cast<BinaryExpression*>(dict->elements[0].value.get());
            ASSERT_NE(binary_expr, nullptr);
            ASSERT_EQ(dict->elements[1].key, "y");

            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_closed_with_wrong_bracket",
            "let a = { x: 1 ]\n"
            "let recovery = 1\n",
            {
            {Err::UnmatchedBraceInDictionaryLiteral, 1, 16}
            },
            ExpectDict({{"x", "1"}})
            },
            ParserErrorsSynchronizationTestCase{
            "empty_dict_with_garbage",
            "let a = { * }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 11} },
            ExpectDict({{"<error>", std::nullopt}})
            },
            ParserErrorsSynchronizationTestCase{
            "dict_nested_brace_overshoot_heuristic",
            "let a = { x: { y: { z: 1 } } } let recovery = 1\n",
            {},
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 1);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto inner_dict = dynamic_cast<DictLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(inner_dict, nullptr);
            ASSERT_EQ(inner_dict->elements.size(), 1);
            EXPECT_EQ(inner_dict->elements[0].key, "y");
            auto inner_inner_dict = dynamic_cast<DictLiteral*>(inner_dict->elements[0].value.get());
            ASSERT_NE(inner_inner_dict, nullptr);
            ASSERT_EQ(inner_inner_dict->elements.size(), 1);
            EXPECT_EQ(inner_inner_dict->elements[0].key, "z");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_nested_brace_broken_value",
            "let a = { x: { y: { z: * } }, w: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 24} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2) << "Both dictionary items should be preserved";

            EXPECT_EQ(dict->elements[0].key, "x");
            auto inner_dict = dynamic_cast<DictLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(inner_dict, nullptr);
            ASSERT_EQ(inner_dict->elements.size(), 1);
            auto inner_inner_dict = dynamic_cast<DictLiteral*>(inner_dict->elements[0].value.get());
            ASSERT_NE(inner_inner_dict, nullptr);
            ASSERT_EQ(inner_inner_dict->elements.size(), 1);


            EXPECT_EQ(dict->elements[1].key, "w");
            auto y_val = dynamic_cast<NumberLiteral*>(dict->elements[1].value.get());
            ASSERT_NE(y_val, nullptr);
            EXPECT_EQ(y_val->value, "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_empty_with_comma",
            "let a = { , }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 11} },
            ExpectDict({{"<error>", std::nullopt}})
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_both_key_and_value",
            "let a = { :, :, z: 3 }\n"
            "let recovery = 1\n",
            {
            {Err::ExpectedDictionaryKey, 1, 11},
            {Err::InvalidExpression, 1, 12},
            {Err::ExpectedDictionaryKey, 1, 14},
            {Err::InvalidExpression, 1, 15},
            },
            ExpectDict({{"<error>", std::nullopt}, {"<error>", std::nullopt}, {"z", "3"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_missing_both_key_and_value_vertical",
            "let a = {\n"
            "   :,\n"
            "   :,\n"
            "   z: 3\n"
            "}\n"
            "let recovery = 1\n",
            {
            {Err::ExpectedDictionaryKey, 2, 4},
            {Err::InvalidExpression,2, 5},
            {Err::ExpectedDictionaryKey, 3, 4},
            {Err::InvalidExpression, 3, 5},
            },
            ExpectDict({{"<error>", std::nullopt}, {"<error>", std::nullopt}, {"z", "3"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_is_number",
            "let a = { 1: 10, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedDictionaryKey, 1, 11} },
            ExpectDict({{"<error>", "10"}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_broken_with_postfix_access",
            "let a = { x: * }.y\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 14} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            auto dot_access = dynamic_cast<DotAccess*>(assign->value.get());
            ASSERT_NE(dot_access, nullptr);
            EXPECT_EQ(dot_access->property_name, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_broken_inside_function_call",
            "let a = f(a: { x: * }, b: 2)\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 19} },[](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            auto func_call = dynamic_cast<FunctionCall*>(assign->value.get());
            ASSERT_NE(func_call, nullptr);
            ASSERT_EQ(func_call->arguments.size(), 2);
            EXPECT_EQ(func_call->arguments[1].first, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_incomplete_property_access",
            "let a = { x: self., y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedPropertyName, 1, 19} },
            ExpectDict({{"x", std::nullopt}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_bracket_unexpected_comma",
            "let a = { x: self[1, ], y: 2 }\n"
            "let recovery = 1\n",
            { {Err::UnexpectedCommaInBracketAccess, 1, 20} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2) << "Both dictionary items should be preserved";

            EXPECT_EQ(dict->elements[0].key, "x");
            auto bracket = dynamic_cast<BracketAccess*>(dict->elements[0].value.get());
            ASSERT_NE(bracket, nullptr) << "BracketAccess should be recovered gracefully";

            ASSERT_NE(dynamic_cast<SelfExpression*>(bracket->target.get()), nullptr) << "Target must be self";
            EXPECT_EQ(bracket->index.get(), nullptr) <<
            "Index should be nullptr because comma aborted the bound parsing";

            EXPECT_EQ(dict->elements[1].key, "y");
            auto y_val = dynamic_cast<NumberLiteral*>(dict->elements[1].value.get());
            ASSERT_NE(y_val, nullptr);
            EXPECT_EQ(y_val->value, "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_method_call_broken_args",
            "let a = { x: self.calc(arg: *), y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 29} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto func_call = dynamic_cast<FunctionCall*>(dict->elements[0].value.get());
            ASSERT_NE(func_call, nullptr) << "FunctionCall should be constructed through recovery";

            auto target_dot = dynamic_cast<DotAccess*>(func_call->target.get());
            ASSERT_NE(target_dot, nullptr);
            EXPECT_EQ(target_dot->property_name, "calc");
            ASSERT_NE(dynamic_cast<SelfExpression*>(target_dot->target.get()), nullptr) << "Dot target must be self";

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_standalone_missing_operator",
            "let a = { x: self 10, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::MissingOperator, 1, 19} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto self_expr = dynamic_cast<SelfExpression*>(dict->elements[0].value.get());
            ASSERT_NE(self_expr, nullptr);

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_key_missing_colon",
            "let a = {x self.a, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedColonAfterDictionaryKey, 1, 12} },
            ExpectDict({ {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_deep_chain_interrupted",
            "let a = { x: self.a.b[ * ], y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 24} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2) <<
            "Both dictionary items should be preserved due to local bracket recovery";

            EXPECT_EQ(dict->elements[0].key, "x");
            auto bracket = dynamic_cast<BracketAccess*>(dict->elements[0].value.get());
            ASSERT_NE(bracket, nullptr);
            EXPECT_EQ(bracket->index.get(), nullptr);

            auto target_b = dynamic_cast<DotAccess*>(bracket->target.get());
            ASSERT_NE(target_b, nullptr);
            EXPECT_EQ(target_b->property_name, "b");

            auto target_a = dynamic_cast<DotAccess*>(target_b->target.get());
            ASSERT_NE(target_a, nullptr);
            EXPECT_EQ(target_a->property_name, "a");

            ASSERT_NE(dynamic_cast<SelfExpression*>(target_a->target.get()), nullptr);

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_in_broken_binary_expr",
            "let a = { x: self.a + *, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 23} },
            ExpectDict({{"x", std::nullopt}, {"y", "2"} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_in_switch_target_error",
            "let a = { x: switch(self.) { case A -> 1 }, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedPropertyName, 1, 26} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto switch_expr = dynamic_cast<SwitchExpression*>(dict->elements[0].value.get());
            ASSERT_NE(switch_expr, nullptr);
            EXPECT_EQ(switch_expr->target.get(), nullptr) << "Switch target should be nullptr due to local recovery";
            ASSERT_EQ(switch_expr->cases.size(), 1);

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_in_tuple_error_recovery",
            "let a = { x: (self.a, *), y: 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 23} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto tuple_expr = dynamic_cast<TupleLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(tuple_expr, nullptr);
            ASSERT_EQ(tuple_expr->elements.size(), 1) << "Tuple should contain only the successfully parsed self.a";

            auto dot_access = dynamic_cast<DotAccess*>(tuple_expr->elements[0].get());
            ASSERT_NE(dot_access, nullptr);
            ASSERT_NE(dynamic_cast<SelfExpression*>(dot_access->target.get()), nullptr);

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_in_nested_dict_error",
            "let a = { x: { inner: self. }, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::ExpectedPropertyName, 1, 29} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto inner_dict = dynamic_cast<DictLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(inner_dict, nullptr);
            EXPECT_EQ(inner_dict->elements.size(), 1);
            EXPECT_EQ(inner_dict->elements[0].key, "inner");
            EXPECT_EQ(dict->elements[1].key, "y");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_self_missing_dot_between_properties",
            "let a = { x: self a, y: 2 }\n"
            "let recovery = 1\n",
            { {Err::MissingOperator, 1, 19} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 2);

            EXPECT_EQ(dict->elements[0].key, "x");
            auto self_expr = dynamic_cast<SelfExpression*>(dict->elements[0].value.get());
            ASSERT_NE(self_expr, nullptr);

            EXPECT_EQ(dict->elements[1].key, "y");
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
