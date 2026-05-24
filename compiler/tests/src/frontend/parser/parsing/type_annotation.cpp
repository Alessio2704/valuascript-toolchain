#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {

    struct TypeAnnotationSadParam {
        std::string test_name;
        std::string source_code;
        E expected_error;
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
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        TypeAnnotationSadPathTest,
        testing::Values(
            TypeAnnotationSadParam{"missing_right_paren_tuple_type", "(scalar, decimal", E::
            UnmatchedParenthesisInTuple},
            TypeAnnotationSadParam{"unmatched_diamond_in_generics", "vector<decimal", E::
            UnmatchedBracketAfterGenericArgs}
        ),
        [](const testing::TestParamInfo<TypeAnnotationSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
