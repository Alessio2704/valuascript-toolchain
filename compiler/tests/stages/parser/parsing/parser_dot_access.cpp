#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/parser/ast.h"
#include "stages/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserDotAccessTestBase : public testing::Test {
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

struct DotAccessHappyParam {
    std::string test_id;
    std::string source_code;
};

class DotAccessHappyPathTest : public ParserDotAccessTestBase,
                                  public testing::WithParamInterface<DotAccessHappyParam> {
};

TEST_P(DotAccessHappyPathTest, ParsesSuccessfully) {
    const DotAccessHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto &assignment = ast->execution_steps[0];
        EXPECT_EQ(assignment->targets.size(), 1);
        EXPECT_NE(assignment->value, nullptr) << "Expected assignment to have a value expression.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    DotAccessHappyPathTest,
    testing::Values(
        DotAccessHappyParam{"simple_access", "let a = model.cagr"},
        DotAccessHappyParam{"deep_chain", "let a = company.department.manager.name"},
        DotAccessHappyParam{"method_chaining", "let a = builder.set_x(x: 1).set_y(y: 2).build()"},
        DotAccessHappyParam{"tensor_mixed", "let a = portfolio.assets[0].risk_profile.score"},
        DotAccessHappyParam{"math_expression", "let a = obj.width * obj.height + obj.depth"},
        DotAccessHappyParam{
        "complex_nested_stress",
        "let a = simulation.run()[0].metrics.get_alpha(risk: 0.05).values[10:20].mean"
        },
        DotAccessHappyParam{"spacing_tolerance", "let a = model  .  cagr"},
        DotAccessHappyParam{"inline_dict_access", "let a = { rate: 0.05, name: \"test\" }.rate"},
        DotAccessHappyParam{"inside_arguments", "let a = compute(x: model.x, y: config.bounds[0].max)"}
    ),
    [](const testing::TestParamInfo<DotAccessHappyParam>& info) {
    return info.param.test_id;
    }
);

struct DotAccessSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class DotAccessSadPathTest : public ParserDotAccessTestBase,
                                public testing::WithParamInterface<DotAccessSadParam> {
};

TEST_P(DotAccessSadPathTest, ThrowsCorrectSyntaxError) {
    const DotAccessSadParam &param = GetParam();

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
    DotAccessSadPathTest,
    testing::Values(
        DotAccessSadParam{"missing_property", "let a = model.", ErrorCode::ExpectedPropertyName},
        DotAccessSadParam{"number_as_property", "let a = model.123", ErrorCode::ExpectedPropertyName},
        DotAccessSadParam{"keyword_as_property", "let a = model.let", ErrorCode::ExpectedPropertyName},
        DotAccessSadParam{"missing_property_deep", "let a = model.assets[0].", ErrorCode::ExpectedPropertyName}
    ),
    [](const testing::TestParamInfo<DotAccessSadParam>& info) {
    return info.param.test_id;
    }
);
