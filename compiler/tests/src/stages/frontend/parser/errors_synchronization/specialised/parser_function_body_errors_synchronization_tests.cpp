#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const FunctionDefinition *ExpectRecoveredFunction(const Program &ast, const std::string &expected_name) {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive at the top level.";

            if (ast.function_definitions.empty()) return nullptr;

            auto it = std::find_if(ast.function_definitions.begin(), ast.function_definitions.end(),
                                   [&](const auto &f) { return f->name == expected_name; });

            if (it == ast.function_definitions.end()) return nullptr;
            return it->get();
        }

        auto ExpectFunctionBodySize(std::string name, size_t expected_body_statements) {
            return [name = std::move(name), expected_body_statements](const Program &ast) {
                auto f = ExpectRecoveredFunction(ast, name);
                ASSERT_NE(f, nullptr) << "Function not found!";
                EXPECT_EQ(f->body.size(), expected_body_statements) << "Function body statement count mismatch!";
            };
        }
    }

    class FunctionBodyParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(FunctionBodyParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionBodyStressTests,
        FunctionBodyParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "garbage_statement_recovers_to_next_inside_block",
            "func test() -> int {\n"
            "    let x = 1 + *\n"
            "    let y = 2\n"
            "}\n"
            "let a = 1\n",
            {{Err::InvalidExpression, 2, 17}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "broken_statement_right_before_closing_brace",
            "func test() -> int {\n"
            "    return 1 + *\n"
            "}\n"
            "let a = 1\n",
            {{Err::InvalidExpression, 2, 16}},
            ExpectFunctionBodySize("test", 0)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_closing_brace_escapes_to_top_level",
            "func test() -> int {\n"
            "    let x = 1\n"
            "let a = 1\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 3, 10}},
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->body.size(), 2);
            EXPECT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_brace_recovers_at_enum",
            "func test() -> int {\n"
            "    let x = 1\n"
            "enum State: int { A = 1 }\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 14}},
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->name, "test");
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

            ASSERT_EQ(ast.enum_definitions.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->name, "State");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_brace_recovers_at_struct_with_modifiers",
            "func test() -> int {\n"
            "    let x = 1\n"
            "@export @packed(align: 4) struct Data { id: int }\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 14}},
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

            ASSERT_EQ(ast.struct_definitions.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->name, "Data");
            EXPECT_EQ(ast.struct_definitions[0]->fields.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->modifiers.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_brace_recovers_at_new_function",
            "func first() -> int {\n"
            "    let x = 1\n"
            "@test @test_1 @test_2(a: 1) func second() -> void {}\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 14}},
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 2);

            EXPECT_EQ(ast.function_definitions[0]->name, "first");
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

            EXPECT_EQ(ast.function_definitions[1]->name, "second");
            EXPECT_EQ(ast.function_definitions[1]->modifiers.size(), 3);
            }
            },

            ParserErrorsSynchronizationTestCase{
            "missing_brace_recovers_at_directive",
            "func test() -> int {\n"
            "    let x = 1\n"
            "#pragma = 1\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 14}},
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

            ASSERT_EQ(ast.directives.size(), 1);
            EXPECT_EQ(ast.directives[0]->name, "pragma");
            }
            },

            ParserErrorsSynchronizationTestCase{
            "missing_brace_recovers_at_import",
            "func test() -> int {\n"
            "    let x = 1\n"
            "import \"module.vs\"\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 14}},
            [](const Program &ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

            ASSERT_EQ(ast.import_statements.size(), 1);
            EXPECT_EQ(ast.import_statements[0]->path, "\"module.vs\"");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_broken_statements_in_body",
            "func test() -> int {\n"
            "    let x = \n"
            "    let y = 1 + *\n"
            "    return \n"
            "}\n"
            "let a = 1\n",
            {
            {Err::MissingValueAfterEquals, 2, 12},
            {Err::InvalidExpression, 3, 17},
            {Err::InvalidExpression, 4, 5}
            },
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_func",
            "func test() -> int {\n"
            "    func nested() -> void {}\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_struct",
            "func test() -> int {\n"
            "    struct Data { id: int }\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_enum",
            "func test() -> int {\n"
            "    enum State: int { A = 1 }\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_broken_enum",
            "func test() -> int {\n"
            "    enum State {}\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_import",
            "func test() -> int {\n"
            "    import \"module.vs\"\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_import_multiple",
            "func test() -> int {\n"
            "    import \"module.vs\"\n"
            "    import \"module.vs\"\n"
            "    import \"module.vs\"\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 2, 5},
            {Err::TopLevelDeclarationNotAllowedHere, 3, 5},
            {Err::TopLevelDeclarationNotAllowedHere, 4, 5},
            },
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "illegal_nested_directive",
            "func test() -> int {\n"
            "    #pragma = 1\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_attached_to_illegal_nested_declaration",
            "func test() -> int {\n"
            "    @export struct Data { id: int }\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {{Err::TopLevelDeclarationNotAllowedHere, 2, 5}},
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_as_variable_name_in_body",
            "func test() -> int {\n"
            "    let true = 1\n"
            "    return true\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 2, 9},
            },
            ExpectFunctionBodySize("test", 2)
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_as_standalone_statement",
            "func test() -> int {\n"
            "    let x = 1\n"
            "    false\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidStandaloneStatement, 3, 5}
            },
            ExpectFunctionBodySize("test", 2)
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_operator_crosses_line_into_next_statement",
            "func test() -> int {\n"
            "    let x = 1 +\n"
            "    let y = 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 2, 15},
            },
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "multi_reassignment_not_supported_aborts_statement",
            "func test() -> int {\n"
            "    x, y = 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::MultiReassignmentNotSupported, 2, 6}
            },
            ExpectFunctionBodySize("test", 0)
            },
            ParserErrorsSynchronizationTestCase{
            "reassignment_to_invalid_lvalue_aborts_statement",
            "func test() -> int {\n"
            "    1 + 1 = 2\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidLeftSideExpressionInReassignment, 2, 11}
            },
            ExpectFunctionBodySize("test", 0)
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_expression_statement_recovers_without_panic",
            "func test() -> int {\n"
            "    @modifier foo()\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ModifiersAttachedToInvalidDeclaration, 2, 5}
            },
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_return_statement_recovers_without_panic",
            "func test() -> int {\n"
            "    @modifier return 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ModifiersAttachedToInvalidDeclaration, 2, 5}
            },
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "broken_function_call_recovers_to_next_statement",
            "func test() -> int {\n"
            "    foo(a: 1 b: 2)\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::MissingCommaSeparatorForArgumentsInFunctionCall, 2, 14}
            },
            ExpectFunctionBodySize("test", 2)
            },

            ParserErrorsSynchronizationTestCase{
            "broken_switch_expression_recovers_to_next_statement",
            "func test() -> int {\n"
            "    let x = switch(v) { case A }\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedRightArrowAfterSwitchCaseIdentifier, 2, 32}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto &func = ast.function_definitions[0];
            ASSERT_EQ(func->name, "test");
            ASSERT_EQ(func->body.size(), 2);
            auto switch_expr_assignment = dynamic_cast<Assignment*>(func->body[0].get());
            auto switch_expr = dynamic_cast<SwitchExpression*>(switch_expr_assignment->value.get());
            ASSERT_NE(switch_expr, nullptr);
            ASSERT_EQ(switch_expr->cases.size(), 1);
            ASSERT_EQ(switch_expr->cases[0].first.size(), 1);
            ASSERT_EQ(switch_expr->cases[0].first[0], "A");
            ASSERT_EQ(switch_expr->cases[0].second, nullptr);
            ASSERT_EQ(switch_expr->default_case, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_modifier_at_end_of_block",
            "func test() -> int {\n"
            "    let x = 1\n"
            "    @\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedModifierName, 3, 6},
            {Err::ModifiersAttachedToInvalidDeclaration, 3, 5}
            },
            ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
            "standalone_math_expression_not_allowed",
            "func test() -> int {\n"
            "    1 + 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::InvalidStandaloneStatement, 2, 9}
            },
            ExpectFunctionBodySize("test", 0)
            },
            ParserErrorsSynchronizationTestCase{
            "missing_comma_in_multi_assignment",
            "func test() -> int {\n"
            "    let x: int y: int = 1\n"
            "    return 1\n"
            "}\n"
            "let a = 1\n",
            {
            {Err::ExpectedCommaInMultiAssignment, 2, 16}
            },
            ExpectFunctionBodySize("test", 2)
            },
            ParserErrorsSynchronizationTestCase{
            "nested_struct_with_proper_closing_brace_stays_in_function",
            "func test() -> int {\n"
            "    let a = 1\n"
            "    struct Nested { id: int }\n"
            "    let b = 2\n"
            "}\n"
            "let c = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 3, 5}
            },
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            EXPECT_EQ(f->body.size(), 2);

            EXPECT_EQ(ast.struct_definitions.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "nested_struct_without_closing_brace_escapes_to_top_level",
            "func test() -> int {\n"
            "    let a = 1\n"
            "    struct TopLevel { id: int }\n"
            "let c = 1\n",
            {
            {Err::ExpectedRightBraceAfterFunctionBody, 2, 14}
            },
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            EXPECT_EQ(f->body.size(), 1);

            ASSERT_EQ(ast.struct_definitions.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->name, "TopLevel");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "interleaved_valid_and_invalid_statements",
            "func test() -> int {\n"
            "    let a = 1\n"
            "    1 + * \n"
            "    let b = 2\n"
            "    foo(x: )\n"
            "    let c = 3\n"
            "}\n"
            "let d = 1\n",
            {
            {Err::InvalidExpression, 3, 9},
            {Err::InvalidExpression, 5, 12}
            },
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->body.size(), 4);

            auto assign1 = dynamic_cast<Assignment*>(f->body[0].get());
            auto assign2 = dynamic_cast<Assignment*>(f->body[1].get());
            auto func_call_expr = dynamic_cast<ExpressionStatement*>(f->body[2].get());
            auto assign3 = dynamic_cast<Assignment*>(f->body[3].get());

            ASSERT_NE(assign1, nullptr);
            EXPECT_EQ(assign1->targets[0].first, "a");
            ASSERT_NE(assign2, nullptr);
            EXPECT_EQ(assign2->targets[0].first, "b");
            ASSERT_NE(func_call_expr, nullptr);
            auto func_call = dynamic_cast<FunctionCall*>(func_call_expr->expr.get());
            ASSERT_NE(func_call, nullptr);
            EXPECT_EQ(func_call->arguments.size(), 1);
            auto func_call_id = dynamic_cast<IdentifierAccess*>(func_call->target.get());
            ASSERT_NE(func_call_id, nullptr);
            EXPECT_EQ(func_call_id->name, "foo");
            ASSERT_NE(assign3, nullptr);
            EXPECT_EQ(assign3->targets[0].first, "c");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dangling_assignment_right_before_closing_brace",
            "func test() -> int {\n"
            "    let a = 1\n"
            "    let b = \n"
            "}\n"
            "let c = 1\n",
            {
            {Err::InvalidExpression, 3, 11}
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
