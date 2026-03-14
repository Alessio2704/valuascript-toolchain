#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct TypeAnnotationHappyParam {
    std::string test_id;
    std::string source_code;
};

class TypeAnnotationHappyPathTest : public test::AstBaseTest,
                                public testing::WithParamInterface<TypeAnnotationHappyParam> {
};

TEST_P(TypeAnnotationHappyPathTest, ParsesSuccessfully) {
    const TypeAnnotationHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_expression_as_type_annotation(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        EXPECT_NE(assignment->value, nullptr) << "Expected assignment to have a value expression.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    TypeAnnotationHappyPathTest,
    testing::Values(
        // actual code being compiler is: let result: 'source code provided below' = 1
        TypeAnnotationHappyParam{"simple", "scalar"},
        TypeAnnotationHappyParam{"template", "vector<scalar>"},
        TypeAnnotationHappyParam{"template_multiple", "vector<vector<decimal>>"},
        TypeAnnotationHappyParam{"tuple", "(decimal, scalar, bool, string)"},
        TypeAnnotationHappyParam{"tuple_complex", "(vector<vector<decimal>>, decimal)"}
    ),
    [](const testing::TestParamInfo<TypeAnnotationHappyParam>& info) {
    return info.param.test_id;
    }
);

struct TypeAnnotationSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class TypeAnnotationSadPathTest : public test::AstBaseTest,
                              public testing::WithParamInterface<TypeAnnotationSadParam> {
};

TEST_P(TypeAnnotationSadPathTest, ThrowsCorrectSyntaxError) {
    const TypeAnnotationSadParam &param = GetParam();

    try {
        parse_expression_as_type_annotation(param.source_code);
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
    TypeAnnotationSadPathTest,
    testing::Values(
    TypeAnnotationSadParam{"empty_generic", "vector<>", ErrorCode::EmptyGenericTypeAnnotation},
    TypeAnnotationSadParam{"missing_comma_generic", "result<scalar decimal>", ErrorCode::ExpectedCommaSeparatorInGenericArgs},
    TypeAnnotationSadParam{"missing_comma_tuple_type", "(scalar decimal)", ErrorCode::ExpectedCommaSeparatorInTupleType},
    TypeAnnotationSadParam{"missing_right_paren_tuple_type", "(scalar, decimal", ErrorCode::UnmatchedParenthesisInTuple},
    TypeAnnotationSadParam{"unmatched_diamond_in_generics", "vector<decimal", ErrorCode::UnmatchedBracket}
    ),
    [](const testing::TestParamInfo<TypeAnnotationSadParam>& info) {
    return info.param.test_id;
    }
);
