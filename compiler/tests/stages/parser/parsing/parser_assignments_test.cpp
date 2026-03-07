#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserAssignmentTestBase : public testing::Test {
protected:
    static std::shared_ptr<Program> parse_code(const std::string &code) {
        LexerStage lexer;
        std::vector<CompilerStageArtifact> lexer_history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        };
        auto lexer_result = lexer.run(lexer_history);

        ParserStage parser;
        std::vector<CompilerStageArtifact> parser_history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        };
        auto parser_result = parser.run(parser_history);

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }
};

struct AssignmentHappyParam {
    std::string test_id;
    std::string source_code;
    size_t expected_target_count;
};

class AssignmentHappyPathTest : public ParserAssignmentTestBase,
                                public testing::WithParamInterface<AssignmentHappyParam> {
};

TEST_P(AssignmentHappyPathTest, ParsesSuccessfully) {
    const AssignmentHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto &assignment = ast->execution_steps[0];
        EXPECT_EQ(assignment->targets.size(), param.expected_target_count);
        EXPECT_NE(assignment->value, nullptr) << "Expected assignment to have a value expression.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    AssignmentHappyPathTest,
    testing::Values(
        AssignmentHappyParam{"number", "let a = 1000", 1},
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
        AssignmentHappyParam{"multi_assignment_params", "let a, b = some_func(a: 1, b: 1.5, c: true, d: \"call\")", 2},
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
        AssignmentHappyParam{"parenthesis_in_assignment", "let a = (x + y) * z", 1}
    ),
    [](const testing::TestParamInfo<AssignmentHappyParam>& info) {
    return info.param.test_id;
    }
);

struct AssignmentSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class AssignmentSadPathTest : public ParserAssignmentTestBase,
                              public testing::WithParamInterface<AssignmentSadParam> {
};

TEST_P(AssignmentSadPathTest, ThrowsCorrectSyntaxError) {
    const AssignmentSadParam &param = GetParam();

    try {
        parse_code(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    AssignmentSadPathTest,
    testing::Values(
        AssignmentSadParam{"missing_let", "x = 1", ErrorCode::UnexpectedToken},
        AssignmentSadParam{"missing_let_multi_assignment", "x, y = some_func()", ErrorCode::UnexpectedToken},
        AssignmentSadParam{"missing_var_name", "let = 1", ErrorCode::InvalidIdentifier},
        AssignmentSadParam{"missing_type_after_colon", "let a: = 1", ErrorCode::MissingTypeAnnotation},
        AssignmentSadParam{"missing_type_after_colon_2", "let a: integer, b:  = test()", ErrorCode::
        MissingTypeAnnotation},
        AssignmentSadParam{"invalid_cname_for_var_name_1", "let 12 = 1", ErrorCode::InvalidIdentifier},
        AssignmentSadParam{"missing_multi_assignment_second_var", "let x, = some_func()", ErrorCode::InvalidIdentifier},
        AssignmentSadParam{"missing_multi_assignment_comma", "let x y = some_func()", ErrorCode::IncompleteAssignment},
        AssignmentSadParam{"missing_value_after_eq_multi_assignment", "let x, y = ", ErrorCode::MissingValueAfterEquals}
        ,
        AssignmentSadParam{"missing_property_on_identifier", "let a = model.", ErrorCode::ExpectedPropertyName},
        AssignmentSadParam{"chaining_not_allowed_for_comparison_1", "let x = a > b > c", ErrorCode::
        ChainingNotAllowedForComparisonOperations},
        AssignmentSadParam{"chaining_not_allowed_for_comparison_2", "let a = 10 <= 5 != false", ErrorCode::
        ChainingNotAllowedForComparisonOperations},
        AssignmentSadParam{"reserved_keyword_1", "let import = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_2", "let let = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_3", "let func = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_4", "let if = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_5", "let then = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_6", "let else = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_7", "let return = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_8", "let struct = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_9", "let true = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_10", "let false = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_11", "let and = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_12", "let or = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_13", "let not = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_14", "let enum = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_15", "let switch = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_16", "let case = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_17", "let default = a", ErrorCode::ReservedKeywordAsIdentifier},
        AssignmentSadParam{"reserved_keyword_multiple", "let x, func = some_func()", ErrorCode::
        ReservedKeywordAsIdentifier}

    ),
    [](const testing::TestParamInfo<AssignmentSadParam>& info) {
    return info.param.test_id;
    }
);
