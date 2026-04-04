#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct DirectiveHappyParam {
        std::string test_name;
        std::string source_code;
        std::string expected_name;
        bool expects_value;
    };

    class DirectiveHappyPathTest : public AstBaseTest,
                                   public testing::WithParamInterface<DirectiveHappyParam> {
    };

    TEST_P(DirectiveHappyPathTest, ParsesSuccessfully) {
        const auto &[test_name, source_code, expected_name, expects_value] = GetParam();

        std::shared_ptr<Program> ast;
        EXPECT_NO_THROW({
            ast = parse_code(source_code);
            }) << "Parser threw an exception on valid directive test: " << test_name;

        if (ast) {
            ASSERT_EQ(ast->directives.size(), 1) << "Expected exactly 1 directive in AST.";
            EXPECT_EQ(ast->execution_steps.size(), 0);
            EXPECT_EQ(ast->function_definitions.size(), 0);

            auto &directive = ast->directives[0];
            EXPECT_EQ(directive->name, expected_name);

            if (expects_value) {
                EXPECT_NE(directive->value, nullptr) << "Expected directive to have a value.";
            } else {
                EXPECT_EQ(directive->value, nullptr) << "Expected directive to be valueless.";
            }
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        DirectiveHappyPathTest,
        testing::Values(
            DirectiveHappyParam{"iterations", "#iterations = 1000", "iterations", true},
            DirectiveHappyParam{"no_equal_1", "#no_equal one", "no_equal", true},
            DirectiveHappyParam{"no_equal_2", "#no_equal 1", "no_equal", true},
            DirectiveHappyParam{"no_equal_3", "#no_equal {a: 1, b: 2}", "no_equal", true},
            DirectiveHappyParam{"no_equal_4", "#no_equal (1, 2, 3)", "no_equal", true},
            DirectiveHappyParam{"no_equal_5", "#no_equal if a then 1 else 2", "no_equal", true},
            DirectiveHappyParam{"output", "#output = my_var", "output", true},
            DirectiveHappyParam{"output_file", "#output_file = \"results.csv\"", "output_file", true},
            DirectiveHappyParam{"module", "#module", "module", false},
            DirectiveHappyParam{"no_value_directive", "#no_value", "no_value", false},
            DirectiveHappyParam{"value_directive_1", "#value = 1", "value", true},
            DirectiveHappyParam{"value_directive_2", "#value = 10.5", "value", true},
            DirectiveHappyParam{"value_directive_3", "#value = 1_000.5", "value", true},
            DirectiveHappyParam{"value_directive_4", "#value = \"string\"", "value", true},
            DirectiveHappyParam{"value_directive_5", "#value = true", "value", true},
            DirectiveHappyParam{"value_directive_6", "#value = var_name", "value", true}
        ),
        [](const testing::TestParamInfo<DirectiveHappyParam>& info) {
        return info.param.test_name;
        }
    );

    struct DirectiveSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class DirectiveSadPathTest : public AstBaseTest,
                                 public testing::WithParamInterface<DirectiveSadParam> {
    };

    TEST_P(DirectiveSadPathTest, ThrowsCorrectSyntaxError) {
        const DirectiveSadParam &param = GetParam();

        try {
            parse_code(param.source_code);
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
        DirectiveSadPathTest,
        testing::Values(
            DirectiveSadParam{"other_from_at", "*iterations = 1000", ValuascriptErrorCode::InvalidExpression},
            DirectiveSadParam{"missing_after_name_at", "@ = 1000", ValuascriptErrorCode::ExpectedModifierName},
            DirectiveSadParam{"missing_value_after_eq", "#iterations = ", ValuascriptErrorCode::MissingValueAfterEquals}
            ,
            DirectiveSadParam{"missing_at_valueless_directive", "module", ValuascriptErrorCode::
            InvalidStandaloneStatement},
            DirectiveSadParam{"other_from_at_valueless_directive", "*module", ValuascriptErrorCode::
            InvalidExpression},
            DirectiveSadParam{"missing_import_directive", "#", ValuascriptErrorCode::MissingDirectiveName},
            DirectiveSadParam{"missing_after_name_at_valueless_directive", "# \"file.vs\"", ValuascriptErrorCode::
            MissingDirectiveName},
            DirectiveSadParam{"missing_operator_1", "#iterations = 1000 1", ValuascriptErrorCode::MissingOperator},
            DirectiveSadParam{"missing_operator_2", "#iterations = 1000 + 1 2", ValuascriptErrorCode::MissingOperator},
            DirectiveSadParam{"missing_operator_3", "#iterations = 1000 + (1 2)", ValuascriptErrorCode::
            MissingOperatorInsideGrouping},
            DirectiveSadParam{"missing_operator_4", "#iterations = 1000  (1 + 2)", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            DirectiveSadParam{"missing_operator_5", "#iterations = 1000 a() + b()", ValuascriptErrorCode::
            MissingOperator}
        ),
        [](const testing::TestParamInfo<DirectiveSadParam>& info) {
        return info.param.test_name;
        }
    );
}
