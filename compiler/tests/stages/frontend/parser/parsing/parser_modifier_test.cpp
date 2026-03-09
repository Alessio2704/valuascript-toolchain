#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

enum class TargetNodeType { Assignment, Function, Struct, Enum };

struct ModifierHappyParam {
    std::string test_name;
    std::string source_code;
    TargetNodeType target_node;
    size_t expected_modifier_count;
    std::string first_modifier_name;
    size_t first_modifier_arg_count;
};

class ModifierHappyPathTest : public testing::TestWithParam<ModifierHappyParam> {
protected:
    std::shared_ptr<Program> parse_code(const std::string& code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        auto parser_result = parser.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        });

        return std::any_cast<std::shared_ptr<Program>>(parser_result.data);
    }

    const std::vector<Modifier>* get_modifiers(const std::shared_ptr<Program>& ast, TargetNodeType type) {
        switch (type) {
            case TargetNodeType::Assignment:
                if (!ast->execution_steps.empty()) {
                    if (const auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get())) {
                        return &assign->modifiers;
                    }
                }
                break;
            case TargetNodeType::Function:
                if (!ast->function_definitions.empty()) return &ast->function_definitions[0]->modifiers;
                break;
            case TargetNodeType::Struct:
                if (!ast->struct_definitions.empty()) return &ast->struct_definitions[0]->modifiers;
                break;
            case TargetNodeType::Enum:
                if (!ast->enum_definitions.empty()) return &ast->enum_definitions[0]->modifiers;
                break;
        }
        return {};
    }
};

TEST_P(ModifierHappyPathTest, ValidatesModifierParsing) {
    auto param = GetParam();
    auto ast = parse_code(param.source_code);

    auto modifiers_ptr = get_modifiers(ast, param.target_node);
    ASSERT_NE(modifiers_ptr, nullptr) << "Failed to extract target node modifiers.";

    const auto& modifiers = *modifiers_ptr;

    ASSERT_EQ(modifiers.size(), param.expected_modifier_count) << "Modifier count mismatch.";

    if (param.expected_modifier_count > 0) {
        EXPECT_EQ(modifiers[0].name, param.first_modifier_name) << "First modifier name mismatch.";
        EXPECT_EQ(modifiers[0].arguments.size(), param.first_modifier_arg_count) << "Argument count mismatch.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ModifierTests,
    ModifierHappyPathTest,
    testing::Values(
        ModifierHappyParam{"SingleModNoArgsLet", "@export let x = 1", TargetNodeType::Assignment, 1, "export", 0},
        ModifierHappyParam{"SingleModNoArgsVar", "@mutable var y = 2", TargetNodeType::Assignment, 1, "mutable", 0},
        ModifierHappyParam{"StackedMods", "@export @memoize let z = 3", TargetNodeType::Assignment, 2, "export", 0},
        ModifierHappyParam{"SingleModOneArg", "@bind(ui: \"slider\") let a = 5", TargetNodeType::Assignment, 1, "bind", 1},
        ModifierHappyParam{"SingleModMultipleArgs", "@range(min: 0, max: 100) let b = 50", TargetNodeType::Assignment, 1, "range", 2},
        ModifierHappyParam{"ModWithComplexMathArg", "@calc(val: 10 * 5 + 2) let c = 1", TargetNodeType::Assignment, 1, "calc", 1},
        ModifierHappyParam{"ModWithTensorArg", "@matrix(shape: [2, 2]) let m = [1, 2, 3, 4]", TargetNodeType::Assignment, 1, "matrix", 1},
        ModifierHappyParam{"StackedMixedMods", "@export @bind(target: \"web\") let r = 0", TargetNodeType::Assignment, 2, "export", 0},
        ModifierHappyParam{"FuncSingleMod", "@inline func fast() -> scalar { return 1 }", TargetNodeType::Function, 1, "inline", 0},
        ModifierHappyParam{"FuncStackedWithArgs", "@export @test(name: \"math_check\") func verify() -> void { }", TargetNodeType::Function, 2, "export", 0},
        ModifierHappyParam{"StructSingleMod", "@packed struct Point { x: int }", TargetNodeType::Struct, 1, "packed", 0},
        ModifierHappyParam{"StructWithArgs", "@serializable(format: \"json\") struct Data { }", TargetNodeType::Struct, 1, "serializable", 1},
        ModifierHappyParam{"EnumSingleMod", "@flags enum Permissions: int { R = 1 }", TargetNodeType::Enum, 1, "flags", 0},
        ModifierHappyParam{"EnumStacked", "@export @hidden enum State: int { A = 0 }", TargetNodeType::Enum, 2, "export", 0},
        ModifierHappyParam{"ModWithEmptyParentheses", "@bind() let x = 1", TargetNodeType::Assignment, 1, "bind", 0}
    ),
    [](const testing::TestParamInfo<ModifierHappyParam>& info) {
        return info.param.test_name;
    }
);

struct ModifierSadParam {
    std::string test_name;
    std::string source_code;
    ErrorCode expected_error;
};

class ModifierSadPathTest : public testing::TestWithParam<ModifierSadParam> {
protected:
    void expect_parser_error(const std::string& code, ErrorCode expected_code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        try {
            parser.run({
                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                lexer_result
            });
            FAIL() << "Expected ValuaScriptException to be thrown, but parsing succeeded.";
        } catch (const ValuaScriptException& e) {
            EXPECT_EQ(e.get_code(), expected_code)
                << "Expected error code " << static_cast<int>(expected_code)
                << " but got " << static_cast<int>(e.get_code())
                << ". Message: " << e.what();
        } catch (const std::exception& e) {
            FAIL() << "Caught unexpected exception type: " << e.what();
        }
    }
};

TEST_P(ModifierSadPathTest, ThrowsCorrectSyntaxError) {
    auto param = GetParam();
    expect_parser_error(param.source_code, param.expected_error);
}

INSTANTIATE_TEST_SUITE_P(
    ModifierErrorTests,
    ModifierSadPathTest,
    testing::Values(
        ModifierSadParam{"ModifierOnReturn", "func foo() -> void { @export return 1 }", ErrorCode::UnexpectedToken},
        ModifierSadParam{"ModifierOnStandaloneExpression", "@export 10 * 5", ErrorCode::UnexpectedToken},
        ModifierSadParam{"ModifierOnReassignment", "let x = 1\n@export x = 2", ErrorCode::UnexpectedToken},
        ModifierSadParam{"MissingModifierName", "@ let x = 1", ErrorCode::ExpectedModifierName},
        ModifierSadParam{"MissingColonInArg", "@bind(ui \"slider\") let x = 1", ErrorCode::MissingColonAfterArgument},
        ModifierSadParam{"MissingArgumentName", "@bind(: \"slider\") let x = 1", ErrorCode::MissingArgumentName},
        ModifierSadParam{"UnclosedParenthesis", "@bind(ui: \"slider\" let x = 1", ErrorCode::UnmatchedParenthesis},
        ModifierSadParam{"DoubleAtSign", "@@export let x = 1", ErrorCode::ExpectedModifierName}
    ),
    [](const testing::TestParamInfo<ModifierSadParam>& info) {
        return info.param.test_name;
    }
);