#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct SwitchHappyParam {
        std::string test_name;
        std::string source_code;
    };

    class SwitchHappyPathTest : public AstBaseTest,
                                public testing::WithParamInterface<SwitchHappyParam> {
    };

    TEST_P(SwitchHappyPathTest, ParsesSuccessfully) {
        const SwitchHappyParam &param = GetParam();

        std::shared_ptr<Program> ast;
        EXPECT_NO_THROW({
            ast = parse_code(param.source_code);
            }) << "Parser threw an exception on valid assignment test: " << param.test_name;

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
        SwitchHappyPathTest,
        testing::Values(
            SwitchHappyParam{"standard_switch", "let a = switch (res) { case UP -> 10 case DOWN -> -15 default -> 20 }"}
            ,
            SwitchHappyParam{"multi_match_cases", "let a = switch (res) { case UP, SUS -> 10 case DOWN -> 0 }"},
            SwitchHappyParam{"no_default", "let a = switch (state) { case OPEN -> 1 case CLOSED -> 0 }"},
            SwitchHappyParam{"complex_target", "let a = switch (get_status(s: p)) { case OK -> 100 }"},
            SwitchHappyParam{"complex_result", "let a = switch (res) { case UP -> base_val * 1.5 default -> 0.0 }"},
            SwitchHappyParam{"inline_argument", "let a = calculate(p: switch (res) { case UP -> 1 default -> 0 })"},
            SwitchHappyParam{"nested_switch",
            "let a = switch (x) { case A -> switch (y) { case B -> 1 default -> 0 } default -> -1 }"}
        ),
        [](const testing::TestParamInfo<SwitchHappyParam>& info) {
        return info.param.test_name;
        }
    );

    struct SwitchSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class SwitchSadPathTest : public AstBaseTest,
                              public testing::WithParamInterface<SwitchSadParam> {
    };

    TEST_P(SwitchSadPathTest, ThrowsCorrectSyntaxError) {
        const SwitchSadParam &param = GetParam();

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
        SwitchSadPathTest,
        testing::Values(
            SwitchSadParam{"missing_left_paren", "let a = switch res) { case UP -> 1 }", ValuascriptErrorCode::
            ExpectedLeftParenAfterSwitch},
            SwitchSadParam{"missing_right_paren", "let a = switch (res { case UP -> 1 }",
            ValuascriptErrorCode::ExpectedRightParenAfterSwitchTarget},
            SwitchSadParam{"missing_left_brace", "let a = switch (res) case UP -> 1 }", ValuascriptErrorCode::
            ExpectedLeftBraceBeforeSwitchBody},
            SwitchSadParam{"missing_right_brace", "let a = switch (res) { case UP -> 1", ValuascriptErrorCode::
            ExpectedRightBraceAfterSwitchBody},
            SwitchSadParam{"number_as_case", "let a = switch (res) { case 1 -> 10 }",
            ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase},
            SwitchSadParam{"string_as_case", "let a = switch (res) { case \"UP\" -> 10 }",
            ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase},
            SwitchSadParam{"expression_as_case", "let a = switch (res) { case a + b -> 10 }",
            ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier},
            SwitchSadParam{"missing_arrow_case", "let a = switch (res) { case UP 10 }",
            ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier},
            SwitchSadParam{"missing_arrow_default", "let a = switch (res) { default 10 }", ValuascriptErrorCode::
            ExpectedRightArrowAfterSwitchCaseIdentifier}
            ,
            SwitchSadParam{"duplicate_default", "let a = switch (res) { default -> 1 default -> 2 }",
            ValuascriptErrorCode::MultipleDefaultCasesInSwitch},
            SwitchSadParam{"assignment_in_body", "let a = switch (res) { let b = 2 }",
            ValuascriptErrorCode::ExpectedCaseOrDefaultInsideSwitchBody},
            SwitchSadParam{"missing_operator_1", "let a = switch (res) { case UP -> 1 2 }", ValuascriptErrorCode::
            MissingOperatorInSwitchCaseResult},
            SwitchSadParam{"missing_operator_2", "let a = switch (res) { case UP -> 1 (2 + 3) }", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            SwitchSadParam{"missing_operator_3", "let a = switch (res) { case UP -> 1 + a() (2 + 3) }",
            ValuascriptErrorCode::MissingOperatorOrArgumentName},
            SwitchSadParam{"missing_operator_4", "let a = switch (res) { case UP -> 1 + a() b() }", ValuascriptErrorCode
            ::MissingOperatorInSwitchCaseResult}

        ),
        [](const testing::TestParamInfo<SwitchSadParam>& info) {
        return info.param.test_name;
        }
    );
}
