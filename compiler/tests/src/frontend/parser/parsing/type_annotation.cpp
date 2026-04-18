#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {

    struct TypeAnnotationSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class TypeAnnotationSadPathTest : public AstBaseTest,
                                      public testing::WithParamInterface<TypeAnnotationSadParam> {
    };

    TEST_P(TypeAnnotationSadPathTest, ThrowsCorrectSyntaxError) {
        const TypeAnnotationSadParam &param = GetParam();

        try {
            parse_expression_as_type_annotation(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
                << "Category mismatch on test: " << param.test_name;
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        TypeAnnotationSadPathTest,
        testing::Values(
            TypeAnnotationSadParam{"empty_generic", "vector<>", ValuascriptErrorCode::EmptyGenericTypeAnnotation},
            TypeAnnotationSadParam{"missing_comma_generic", "result<scalar decimal>", ValuascriptErrorCode::
            ExpectedCommaSeparatorInGenericArgs},
            TypeAnnotationSadParam{"missing_comma_tuple_type", "(scalar decimal)", ValuascriptErrorCode::
            ExpectedCommaSeparatorInTupleType},
            TypeAnnotationSadParam{"missing_right_paren_tuple_type", "(scalar, decimal", ValuascriptErrorCode::
            UnmatchedParenthesisInTuple},
            TypeAnnotationSadParam{"unmatched_diamond_in_generics", "vector<decimal", ValuascriptErrorCode::
            UnmatchedBracketAfterGenericArgs}
        ),
        [](const testing::TestParamInfo<TypeAnnotationSadParam>& info) {
        return info.param.test_name;
        }
    );
}
