#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/parser/ast.h"
#include "stages/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserMemberAccessTestBase : public testing::Test {
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

struct MemberAccessHappyParam {
    std::string test_id;
    std::string source_code;
};

class MemberAccessHappyPathTest : public ParserMemberAccessTestBase,
                                  public testing::WithParamInterface<MemberAccessHappyParam> {
};

TEST_P(MemberAccessHappyPathTest, ParsesSuccessfully) {
    const MemberAccessHappyParam &param = GetParam();

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
    MemberAccessHappyPathTest,
    testing::Values(
        MemberAccessHappyParam{"simple_access", "let a = model.cagr"},
        MemberAccessHappyParam{"deep_chain", "let a = company.department.manager.name"},
        MemberAccessHappyParam{"method_chaining", "let a = builder.set_x(x: 1).set_y(y: 2).build()"},
        MemberAccessHappyParam{"tensor_mixed", "let a = portfolio.assets[0].risk_profile.score"},
        MemberAccessHappyParam{"math_expression", "let a = obj.width * obj.height + obj.depth"},
        MemberAccessHappyParam{
        "complex_nested_stress",
        "let a = simulation.run()[0].metrics.get_alpha(risk: 0.05).values[10:20].mean"
        },
        MemberAccessHappyParam{"spacing_tolerance", "let a = model  .  cagr"},
        MemberAccessHappyParam{"inline_dict_access", "let a = { rate: 0.05, name: \"test\" }.rate"},
        MemberAccessHappyParam{"inside_arguments", "let a = compute(x: model.x, y: config.bounds[0].max)"}
    ),
    [](const testing::TestParamInfo<MemberAccessHappyParam>& info) {
    return info.param.test_id;
    }
);

struct MemberAccessSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class MemberAccessSadPathTest : public ParserMemberAccessTestBase,
                                public testing::WithParamInterface<MemberAccessSadParam> {
};

TEST_P(MemberAccessSadPathTest, ThrowsCorrectSyntaxError) {
    const MemberAccessSadParam &param = GetParam();

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
    MemberAccessSadPathTest,
    testing::Values(
        MemberAccessSadParam{"missing_property", "let a = model.", ErrorCode::ExpectedPropertyName},
        MemberAccessSadParam{"number_as_property", "let a = model.123", ErrorCode::ExpectedPropertyName},
        MemberAccessSadParam{"keyword_as_property", "let a = model.let", ErrorCode::ExpectedPropertyName},
        MemberAccessSadParam{"missing_property_deep", "let a = model.assets[0].", ErrorCode::ExpectedPropertyName}
    ),
    [](const testing::TestParamInfo<MemberAccessSadParam>& info) {
    return info.param.test_id;
    }
);
