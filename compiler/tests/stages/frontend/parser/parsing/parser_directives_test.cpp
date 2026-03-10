#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct DirectiveHappyParam {
    std::string test_id;
    std::string source_code;
    std::string expected_name;
    bool expects_value;
};

class DirectiveHappyPathTest : public test::AstBaseTest,
                               public testing::WithParamInterface<DirectiveHappyParam> {};

TEST_P(DirectiveHappyPathTest, ParsesSuccessfully) {
    const auto&[test_id, source_code, expected_name, expects_value] = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(source_code);
    }) << "Parser threw an exception on valid directive test: " << test_id;

    if (ast) {
        ASSERT_EQ(ast->directives.size(), 1) << "Expected exactly 1 directive in AST.";
        EXPECT_EQ(ast->execution_steps.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto& directive = ast->directives[0];
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
        return info.param.test_id;
    }
);

struct DirectiveSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class DirectiveSadPathTest : public test::AstBaseTest,
                             public testing::WithParamInterface<DirectiveSadParam> {};

TEST_P(DirectiveSadPathTest, ThrowsCorrectSyntaxError) {
    const DirectiveSadParam& param = GetParam();

    try {
        parse_code(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException& e) {
        EXPECT_EQ(e.get_category(), ErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    DirectiveSadPathTest,
    testing::Values(
        DirectiveSadParam{"other_from_at", "*iterations = 1000", ErrorCode::UnexpectedToken},
        DirectiveSadParam{"missing_after_name_at", "@ = 1000", ErrorCode::ExpectedModifierName},
        DirectiveSadParam{"missing_value_after_eq", "#iterations = ", ErrorCode::MissingValueAfterEquals},
        DirectiveSadParam{"missing_at_valueless_directive", "module", ErrorCode::InvalidStandaloneStatement},
        DirectiveSadParam{"other_from_at_valueless_directive", "*module", ErrorCode::UnexpectedToken},
        DirectiveSadParam{"missing_import_directive", "#", ErrorCode::MissingDirectiveName},
        DirectiveSadParam{"missing_after_name_at_valueless_directive", "# \"file.vs\"", ErrorCode::MissingDirectiveName}
    ),
    [](const testing::TestParamInfo<DirectiveSadParam>& info) {
        return info.param.test_id;
    }
);