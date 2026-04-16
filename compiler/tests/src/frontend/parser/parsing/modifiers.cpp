#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct ModifierHappyParam {
        std::string test_name;
        std::string source_code;
        TargetNodeType target_node;
        size_t expected_modifier_count;
        std::string first_modifier_name;
        size_t first_modifier_arg_count;
    };

    class ModifierHappyPathTest : public AstBaseTest,
                                  public testing::WithParamInterface<ModifierHappyParam> {
    };


    TEST_P(ModifierHappyPathTest, ValidatesModifierParsing) {
        auto param = GetParam();
        auto ast = parse_code(param.source_code);

        auto modifiers_ptr = get_modifiers(ast, param.target_node);
        ASSERT_NE(modifiers_ptr, nullptr) << "Failed to extract target node modifiers.";

        const auto &modifiers = *modifiers_ptr;

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
            ModifierHappyParam{"StackedMods", "@export @memoize let z = 3", TargetNodeType::Assignment, 2, "export", 0},
            ModifierHappyParam{"SingleModOneArg", "@bind(ui: \"slider\") let a = 5", TargetNodeType::Assignment, 1,
            "bind", 1},
            ModifierHappyParam{"SingleModMultipleArgs", "@range(min: 0, max: 100) let b = 50", TargetNodeType::
            Assignment, 1, "range", 2},
            ModifierHappyParam{"ModWithComplexMathArg", "@calc(val: 10 * 5 + 2) let c = 1", TargetNodeType::Assignment,
            1, "calc", 1},
            ModifierHappyParam{"ModWithTensorArg", "@matrix(shape: [2, 2]) let m = [1, 2, 3, 4]", TargetNodeType::
            Assignment, 1, "matrix", 1},
            ModifierHappyParam{"StackedMixedMods", "@export @bind(target: \"web\") let r = 0", TargetNodeType::
            Assignment, 2, "export", 0},
            ModifierHappyParam{"ModWithEmptyParentheses", "@bind() let x = 1", TargetNodeType::Assignment, 1, "bind", 0}
            ,
            ModifierHappyParam{"FuncSingleMod", "@inline func fast() -> scalar { return 1 }", TargetNodeType::Function,
            1, "inline", 0},
            ModifierHappyParam{"FuncStackedWithArgs", "@export @test(name: \"math_check\") func verify() -> void { }",
            TargetNodeType::Function, 2, "export", 0},
            ModifierHappyParam{"StructSingleMod", "@packed struct Point { x: int }", TargetNodeType::Struct, 1, "packed"
            , 0},
            ModifierHappyParam{"StructWithArgs", "@serializable(format: \"json\") struct Data { }", TargetNodeType::
            Struct, 1, "serializable", 1},
            ModifierHappyParam{"EnumSingleMod", "@flags enum Permissions: int { R = 1 }", TargetNodeType::Enum, 1,
            "flags", 0},
            ModifierHappyParam{"EnumStacked", "@export @hidden enum State: int { A = 0 }", TargetNodeType::Enum, 2,
            "export", 0},
            ModifierHappyParam{"DictKeySingleMod", "let d = { @optional key: 1 }", TargetNodeType::DictKey, 1,
            "optional", 0},
            ModifierHappyParam{"DictKeyStackedMods", "let d = { @root @transient key: 1 }", TargetNodeType::DictKey, 2,
            "root", 0},
            ModifierHappyParam{"DictKeyModWithArgs", "let d = { @rename(to: \"id\") key: 1 }", TargetNodeType::DictKey,
            1, "rename", 1},
            ModifierHappyParam{"DictKeyModEmptyParens", "let d = { @ignore() key: 1 }", TargetNodeType::DictKey, 1,
            "ignore", 0},
            ModifierHappyParam{"EnumCaseSingleMod", "enum E: int { @deprecated A = 0 }", TargetNodeType::EnumCase, 1,
            "deprecated", 0},
            ModifierHappyParam{"EnumCaseStackedMods", "enum E: int { @hidden @internal A = 0 }", TargetNodeType::
            EnumCase, 2, "hidden", 0},
            ModifierHappyParam{"EnumCaseModWithArgs", "enum E: int { @alias(name: \"Old\") A = 0 }", TargetNodeType::
            EnumCase, 1, "alias", 1},
            ModifierHappyParam{"EnumCaseModEmptyParens", "enum E: int { @init() A = 0 }", TargetNodeType::EnumCase, 1,
            "init", 0},
            ModifierHappyParam{"FuncParamNoMods", "func foo(a: int) -> void {}", TargetNodeType::FunctionParameter, 0,
            "", 0},
            ModifierHappyParam{"FuncParamSingleMod", "func foo(@mut a: int) -> void {}", TargetNodeType::
            FunctionParameter, 1, "mut", 0},
            ModifierHappyParam{"FuncParamStackedMods", "func foo(@mut @ref a: int) -> void {}", TargetNodeType::
            FunctionParameter, 2, "mut", 0},
            ModifierHappyParam{"FuncParamModWithArgs", "func foo(@clamp(min: 0, max: 10) a: int) -> void {}",
            TargetNodeType::FunctionParameter, 1, "clamp", 2},
            ModifierHappyParam{"FuncParamModEmptyParens", "func foo(@inject() a: int) -> void {}", TargetNodeType::
            FunctionParameter, 1, "inject", 0},
            ModifierHappyParam{"StructFieldSingleMod", "struct S { @id id: int }", TargetNodeType::StructField, 1, "id",
            0},
            ModifierHappyParam{"StructFieldStackedMods", "struct S { @root @primary id: int }", TargetNodeType::
            StructField, 2, "root", 0},
            ModifierHappyParam{"StructFieldModWithArgs", "struct S { @json(name: \"user_id\") id: int }", TargetNodeType
            ::StructField, 1, "json", 1},
            ModifierHappyParam{"StructFieldModWithComplexMath", "struct S { @clamp(limit: 10 * 10) val: int }",
            TargetNodeType::StructField, 1, "clamp", 1},
            ModifierHappyParam{"StructFieldModEmptyParens", "struct S { @internal() val: int }", TargetNodeType::
            StructField, 1, "internal", 0},
            ModifierHappyParam{"StructFieldModWithTensor", "struct S { @matrix(shape: [3, 1]) v: vec }", TargetNodeType
            ::StructField, 1, "matrix", 1}
        ),
        [](const testing::TestParamInfo<ModifierHappyParam>& info) {
        return info.param.test_name;
        }
    );

    struct ModifierSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class ModifierSadPathTest : public AstBaseTest,
                                public testing::WithParamInterface<ModifierSadParam> {
    };

    TEST_P(ModifierSadPathTest, ThrowsCorrectSyntaxError) {
        auto param = GetParam();
        try {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax);
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ModifierErrorTests,
        ModifierSadPathTest,
        testing::Values(
            ModifierSadParam{"ModifierOnReturn", "func foo() -> void { @export return 1 }", ValuascriptErrorCode::
            ModifiersAttachedToInvalidDeclaration},
            ModifierSadParam{"ModifierOnStandaloneExpression", "@export 10 * 5", ValuascriptErrorCode::
            ModifiersAttachedToInvalidDeclaration},
            ModifierSadParam{"ModifierOnReassignment", "let x = 1\n@export x = 2", ValuascriptErrorCode::
            ModifiersAttachedToInvalidDeclaration},
            ModifierSadParam{"MissingModifierName", "@ let x = 1", ValuascriptErrorCode::ExpectedModifierName},
            ModifierSadParam{"MissingColonInArg", "@bind(ui \"slider\") let x = 1", ValuascriptErrorCode::
            MissingColonAfterArgument},
            ModifierSadParam{"MissingArgumentName", "@bind(: \"slider\") let x = 1", ValuascriptErrorCode::
            MissingArgumentNameInModifier},
            ModifierSadParam{"UnclosedParenthesis", "@bind(ui: \"slider\" let x = 1", ValuascriptErrorCode::
            UnmatchedParenthesisAfterModifierArgs},
            ModifierSadParam{"DoubleAtSign", "@@export let x = 1", ValuascriptErrorCode::ExpectedModifierName},
            ModifierSadParam{"missing_comma_in_param", "@export(a: 1 b: 2) let x = 1", ValuascriptErrorCode::
            MissingCommaSeparatorForArgumentsInModifier},
            ModifierSadParam{"missing_operator_1", "@export(a: 1 2) let x = 1", ValuascriptErrorCode::MissingOperator},
            ModifierSadParam{"missing_operator_2", "@export(a: 1 + 2 3) let x = 1", ValuascriptErrorCode::
            MissingOperator},
            ModifierSadParam{"missing_operator_3", "@export(a: 1 + (2 3)) let x = 1", ValuascriptErrorCode::
            MissingOperatorInsideGrouping},
            ModifierSadParam{"missing_operator_4", "@export(a: 1 (2 + 3)) let x = 1", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            ModifierSadParam{"missing_operator_5", "@export(a: 1 a() + b()) let x = 1", ValuascriptErrorCode::
            MissingOperator},
            ModifierSadParam{"ModifierOnStructFieldType", "struct S { name: @mut string }", ValuascriptErrorCode::
            MissingTypeAnnotation},
            ModifierSadParam{"StructFieldModifierOnClosingBrace", "struct S { @ }", ValuascriptErrorCode::
            ExpectedModifierName}
        ),
        [](const testing::TestParamInfo<ModifierSadParam>& info) {
        return info.param.test_name;
        }
    );
}
