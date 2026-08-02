#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        struct ExpectedDictPair
        {
            std::string key;
            std::optional<std::string> expected_number_value = std::nullopt;
        };

        void ExpectDictLiteral(const Program& ast, const std::vector<ExpectedDictPair>& expected)
        {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto* dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr) << "Assignment value is not a DictLiteral.";

            ASSERT_EQ(dict->elements.size(), expected.size()) << "Dictionary pair count mismatch!";

            for (size_t i = 0; i < expected.size(); ++i)
            {
                EXPECT_EQ(dict->elements[i].key, expected[i].key) << "Dict key mismatch at index " << i;

                if (expected[i].expected_number_value.has_value())
                {
                    auto* num = dynamic_cast<NumberLiteral*>(dict->elements[i].value.get());
                    ASSERT_NE(num, nullptr) << "Expected number literal for key: " << expected[i].key;
                    EXPECT_EQ(num->value, *expected[i].expected_number_value)
                        << "Value mismatch for key: " << expected[i].key;
                }
                else
                {
                    EXPECT_EQ(dict->elements[i].value, nullptr);
                }
            }
        }

        auto ExpectDict(std::vector<ExpectedDictPair> pairs)
        {
            return [p = std::move(pairs)](const Program& ast)
            {
                ExpectDictLiteral(ast, p);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

    class DictLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(DictLiteralParserSynchronizationTest, SynchronizesDictLiteralErrors)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        DictLiteralStressTests,
        DictLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_missing_closing_brace",
                .source_code = "let a = { x: 1 \nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 14} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_missing_multiple_closing_brace",
                .source_code = "let a = { x: 1 \nlet b = { x: 1 \nlet c = { x: 1 \nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 14},
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 2, .column = 14},
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 3, .column = 14}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 4);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "array_inside_dict_with_error",
                .source_code = "let a = {a: [1, 2}\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 17} },
                .verify_ast = [](const Program& ast) {
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
                .test_name = "dict_total_mangle",
                .source_code = "let a = { x: 1, let y = 2\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::TopLevelDeclarationNotAllowedHere, .line = 1, .column = 17},
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 25}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_total_mangle_new_line",
                .source_code = "let a = { x: 1,\nlet y = 2\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 15} },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 3);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_eof_after_key",
                .source_code = "let a = { x\n",
                .expected_errors = {
                    {.code = Err::ExpectedColonAfterDictionaryKey, .line = 1, .column = 12},
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 11}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
                    ASSERT_NE(dict, nullptr);
                    EXPECT_EQ(dict->elements.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_eof_after_colon",
                .source_code = "let a = { x: \n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 1, .column = 13},
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 12}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                    ExpectDictLiteral(ast, {{.key = "x", .expected_number_value = std::nullopt}});
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_eof_after_comma",
                .source_code = "let a = { x: 1, \n",
                .expected_errors = {
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 15}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 1);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
                    ASSERT_NE(dict, nullptr);
                    EXPECT_EQ(dict->elements.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_closed_with_wrong_bracket",
                .source_code = "let a = { x: 1 ]\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBraceInDictionaryLiteral, .line = 1, .column = 14}
                },
                .verify_ast = ExpectDict({{.key = "x", .expected_number_value = "1"}})
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_self_bracket_unexpected_comma",
                .source_code = "let a = { x: self[1, ], y: 2 }\nlet recovery = 1\n",
                .expected_errors = { {.code = Err::UnexpectedCommaInBracketAccess, .line = 1, .column = 20} },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
                    ASSERT_NE(dict, nullptr);
                    ASSERT_EQ(dict->elements.size(), 2) << "Both dictionary items should be preserved";

                    EXPECT_EQ(dict->elements[0].key, "x");
                    auto bracket = dynamic_cast<BracketAccess*>(dict->elements[0].value.get());
                    ASSERT_NE(bracket, nullptr) << "BracketAccess should be recovered gracefully";

                    ASSERT_NE(dynamic_cast<SelfExpression*>(bracket->target.get()), nullptr) << "Target must be self";
                    EXPECT_NE(bracket->index.get(), nullptr) <<
                        "Index should be recovered because comma aborts parsing after yielding the bound";

                    EXPECT_EQ(dict->elements[1].key, "y");
                    auto y_val = dynamic_cast<NumberLiteral*>(dict->elements[1].value.get());
                    ASSERT_NE(y_val, nullptr);
                    EXPECT_EQ(y_val->value, "2");
                }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}
