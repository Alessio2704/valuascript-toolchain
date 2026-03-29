#include <gtest/gtest.h>

#include <utility>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct AssignmentHappyParam {
        std::string test_name;
        std::string source_code;
        size_t expected_target_count;
    };

    class AssignmentHappyPathTest : public AstBaseTest,
                                    public testing::WithParamInterface<AssignmentHappyParam> {
    };

    TEST_P(AssignmentHappyPathTest, ParsesSuccessfully) {
        const AssignmentHappyParam &param = GetParam();

        std::shared_ptr<Program> ast;
        EXPECT_NO_THROW({
            ast = parse_code(param.source_code);
            }) << "Parser threw an exception on valid assignment test: " << param.test_name;

        if (ast) {
            ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
            EXPECT_EQ(ast->directives.size(), 0);
            EXPECT_EQ(ast->function_definitions.size(), 0);

            auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
            EXPECT_EQ(assignment->targets.size(), param.expected_target_count);
            EXPECT_NE(assignment->value, nullptr) << "Expected assignment to have a value expression.";
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        AssignmentHappyPathTest,
        testing::Values(
            AssignmentHappyParam{"number", "let a = 1000", 1},
            AssignmentHappyParam{"number_percentage_1", "let a = 1.5%", 1},
            AssignmentHappyParam{"number_percentage_2", "let a = 0.000_001%", 1},
            AssignmentHappyParam{"number_explicit_type", "let a: integer = 1000", 1},
            AssignmentHappyParam{"explicit_type_tuple_definition", "let a: (integer, string) = test()", 1},
            AssignmentHappyParam{"explicit_type_tuple_definition_2", "let a: (integer, Model) = test()", 1},
            AssignmentHappyParam{"explicit_type_tuple_definition_3", "let a: (Vector<integer>, string) = test()", 1},
            AssignmentHappyParam{"explicit_type_generic", "let a: Vector<integer> = [1000]", 1},
            AssignmentHappyParam{"explicit_type_multiple", "let a: Vector<integer>, b: string = test()", 2},
            AssignmentHappyParam{"explicit_type_multiple_2", "let a, b: string = test()", 2},
            AssignmentHappyParam{"explicit_type_multiple_3", "let a: bool, b = test()", 2},
            AssignmentHappyParam{"string", "let a = \"string\"", 1},
            AssignmentHappyParam{"boolean", "let a = true", 1},
            AssignmentHappyParam{"cname_3", "let _a = false", 1},
            AssignmentHappyParam{"identifier_containing_keyword", "let ifthenelse = 1", 1},
            AssignmentHappyParam{"multi_assignment", "let a, b = some_func()", 2},
            AssignmentHappyParam{"multi_assignment_params", "let a, b = some_func(a: 1, b: 1.5, c: true, d: \"call\")",
            2},
            AssignmentHappyParam{"conditional_expression", "let a = if true then 10 else 4", 1},
            AssignmentHappyParam{"or_expr", "let a = x or y", 1},
            AssignmentHappyParam{"and_expr", "let a = x and y", 1},
            AssignmentHappyParam{"not_expr", "let a = not x", 1},
            AssignmentHappyParam{"eq_expr", "let a = x == y", 1},
            AssignmentHappyParam{"neq_expr", "let a = x != y", 1},
            AssignmentHappyParam{"gt_expr", "let a = x > y", 1},
            AssignmentHappyParam{"lt_expr", "let a = x < y", 1},
            AssignmentHappyParam{"gte_expr", "let a = x >= y", 1},
            AssignmentHappyParam{"lte_expr", "let a = x <= y", 1},
            AssignmentHappyParam{"pow_expr", "let a = x^y", 1},
            AssignmentHappyParam{"parenthesis_in_assignment", "let a = (x + y) * z", 1},
            AssignmentHappyParam{"assignment_with_decorator", "@export let a = (x + y) * z", 1},
            AssignmentHappyParam{"assignment_with_decorator_multiple", "@export let a, b = (1, 2)", 2}
        ),
        [](const testing::TestParamInfo<AssignmentHappyParam>& info) {
        return info.param.test_name;
        }
    );

    struct AssignmentSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
        ValuascriptErrorCategory expected_error_category;

        AssignmentSadParam(std::string test_name,
                           std::string source_code,
                           ValuascriptErrorCode expected_error,
                           ValuascriptErrorCategory expected_error_category =
                                   ValuascriptErrorCategory::Syntax) : test_name(std::move(test_name)),
                                                                       source_code(std::move(source_code)),
                                                                       expected_error(expected_error),
                                                                       expected_error_category(
                                                                           expected_error_category) {
        };
    };

    class AssignmentSadPathTest : public AstBaseTest,
                                  public testing::WithParamInterface<AssignmentSadParam> {
    };

    TEST_P(AssignmentSadPathTest, ThrowsCorrectSyntaxError) {
        const AssignmentSadParam &param = GetParam();

        try {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), param.expected_error_category)
                << "Category mismatch on test: " << param.test_name;
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        AssignmentSadPathTest,
        testing::Values(
            AssignmentSadParam{"invalid_character_1", "let a! = 1", ValuascriptErrorCode::InvalidCharacter,
            ValuascriptErrorCategory::Lexical},
            AssignmentSadParam{"invalid_character_2", "let a ! = 1", ValuascriptErrorCode::InvalidCharacter,
            ValuascriptErrorCategory::Lexical},
            AssignmentSadParam{"missing_var_name", "let = 1", ValuascriptErrorCode::InvalidIdentifier},
            AssignmentSadParam{"missing_type_after_colon", "let a: = 1", ValuascriptErrorCode::MissingTypeAnnotation},
            AssignmentSadParam{"missing_type_after_colon_2", "let a: integer, b:  = test()", ValuascriptErrorCode::
            MissingTypeAnnotation},
            AssignmentSadParam{"invalid_cname_for_var_name_1", "let 12 = 1", ValuascriptErrorCode::InvalidIdentifier},
            AssignmentSadParam{"missing_multi_assignment_second_var", "let x, = some_func()", ValuascriptErrorCode::
            InvalidIdentifier},
            AssignmentSadParam{"missing_multi_assignment_comma", "let x y = some_func()", ValuascriptErrorCode::
            ExpectedCommaInMultiAssignment},
            AssignmentSadParam{"missing_value_after_eq_multi_assignment", "let x, y = ", ValuascriptErrorCode::
            MissingValueAfterEquals}
            ,
            AssignmentSadParam{"missing_property_on_identifier", "let a = model.", ValuascriptErrorCode::
            ExpectedPropertyName},
            AssignmentSadParam{"missing_operator_1", "let a = a + b c", ValuascriptErrorCode::MissingOperator},
            AssignmentSadParam{"missing_operator_2", "let a = a + b (1 + 2)", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            AssignmentSadParam{"missing_operator_3", "let a = a + b model.a", ValuascriptErrorCode::MissingOperator},
            AssignmentSadParam{"missing_operator_4", "let a = a + b vec[0]", ValuascriptErrorCode::MissingOperator},
            AssignmentSadParam{"missing_operator_5", "let a = a + b {}", ValuascriptErrorCode::MissingOperator},
            AssignmentSadParam{"missing_operator_6", "let a = a  b[]", ValuascriptErrorCode::MissingOperator},
            AssignmentSadParam{"missing_operator_6_a", "let a = a - b[]", ValuascriptErrorCode::EmptyBracketAccess},
            AssignmentSadParam{"missing_operator_7", "let a = a + b (1, 2)", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            AssignmentSadParam{"missing_modifier_name", "@ let a = model", ValuascriptErrorCode::ExpectedModifierName},
            AssignmentSadParam{"chaining_not_allowed_for_comparison_1", "let x = a > b > c", ValuascriptErrorCode::
            ChainingNotAllowedForComparisonOperations},
            AssignmentSadParam{"chaining_not_allowed_for_comparison_2", "let a = 10 <= 5 != false", ValuascriptErrorCode
            ::
            ChainingNotAllowedForComparisonOperations},
            AssignmentSadParam{"reserved_keyword_1", "let import = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier
            },
            AssignmentSadParam{"reserved_keyword_2", "let let = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_3", "let func = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_4", "let if = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_5", "let then = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_6", "let else = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_7", "let return = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier
            },
            AssignmentSadParam{"reserved_keyword_8", "let struct = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier
            },
            AssignmentSadParam{"reserved_keyword_9", "let true = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_10", "let false = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier
            },
            AssignmentSadParam{"reserved_keyword_11", "let and = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_12", "let or = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_13", "let not = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_14", "let enum = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier}
            ,
            AssignmentSadParam{"reserved_keyword_15", "let switch = a", ValuascriptErrorCode::
            ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_16", "let case = a", ValuascriptErrorCode::ReservedKeywordAsIdentifier}
            ,
            AssignmentSadParam{"reserved_keyword_17", "let default = a", ValuascriptErrorCode::
            ReservedKeywordAsIdentifier},
            AssignmentSadParam{"reserved_keyword_multiple", "let x, func = some_func()", ValuascriptErrorCode::
            ReservedKeywordAsIdentifier}

        ),
        [](const testing::TestParamInfo<AssignmentSadParam>& info) {
        return info.param.test_name;
        }
    );
}
