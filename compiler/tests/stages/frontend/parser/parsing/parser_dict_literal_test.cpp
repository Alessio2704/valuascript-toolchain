#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::shared;
using namespace valuascript::compiler;

struct DictLiteralHappyParam {
    std::string test_id;
    std::string source_code;
};

class DictLiteralHappyPathTest : public test::AstBaseTest,
                                 public testing::WithParamInterface<DictLiteralHappyParam> {
};

TEST_P(DictLiteralHappyPathTest, ParsesSuccessfully) {
    const DictLiteralHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_expression_as_assignment(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        EXPECT_EQ(assignment->targets.size(), 1);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    DictLiteralHappyPathTest,
    testing::Values(
        DictLiteralHappyParam{"dict", "{ name: \"one\", age: 20 }"},
        DictLiteralHappyParam{"dict_trailing_comma", "{ name: \"one\", age: 20, }"},
        DictLiteralHappyParam{"dict_complex", "{ name: func_call(), age: 20 }"},
        DictLiteralHappyParam{"dict_complex_1", "{ name: func_call(), age: matrix[0][:] }"},
        DictLiteralHappyParam{"dict_complex_2",
        "{ name: func_call(), age: matrix[0][:], money: if a() then b else if c() then d(c: 12) else 10 }"},
        DictLiteralHappyParam{"dict_complex_3",
        "{ name: \"one\", age: 20, assets: [one, two, three], obj: {a: 1, b: 2} }"}
    ),
    [](const testing::TestParamInfo<DictLiteralHappyParam>& info) {
    return info.param.test_id;
    }
);

struct DictLiteralSadParam {
    std::string test_id;
    std::string source_code;
    ValuascriptErrorCode expected_error;
};

class DictLiteralSadPathTest : public test::AstBaseTest,
                               public testing::WithParamInterface<DictLiteralSadParam> {
};

TEST_P(DictLiteralSadPathTest, ThrowsCorrectSyntaxError) {
    const DictLiteralSadParam &param = GetParam();

    try {
        parse_expression_as_assignment(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    DictLiteralSadPathTest,
    testing::Values(
        DictLiteralSadParam{"dict_missing_brace", "{a: 1", ValuascriptErrorCode::
        UnmatchedBraceInDictionaryLiteral},
        DictLiteralSadParam{"dict_missing_comma", "{a: 1 b: 2}", ValuascriptErrorCode::
        ExpectedCommaSeparatorInDictionaryLiteral},
        DictLiteralSadParam{"dict_missing_key", "{1}", ValuascriptErrorCode::
        ExpectedDictionaryKey},
        DictLiteralSadParam{"dict_missing_colon", "{a 1}", ValuascriptErrorCode::
        ExpectedColonAfterDictionaryKey},
        DictLiteralSadParam{"dict_empty", "{a}", ValuascriptErrorCode::
        ExpectedColonAfterDictionaryKey},
        DictLiteralSadParam{"dict_key_string_literal", "{ \"key\" 10 }", ValuascriptErrorCode::ExpectedDictionaryKey},
        DictLiteralSadParam{"dict_key_missing_operator", "{ market_size: 13_624 / 11%   4, }", ValuascriptErrorCode::MissingOperator}
    ),
    [](const testing::TestParamInfo<DictLiteralSadParam>& info) {
    return info.param.test_id;
    }
);
