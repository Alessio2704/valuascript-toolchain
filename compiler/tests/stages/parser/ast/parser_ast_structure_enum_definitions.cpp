#include <gtest/gtest.h>

#include "stages/parser/parser_stage.h"
#include "stages/lexer/lexer_stage.h"
#include "stages/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstEnumDefinitionTest : public testing::Test {
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

    EnumDefinition* get_first_enum(const std::shared_ptr<Program>& ast) {
        if (ast->enum_definitions.empty()) return nullptr;
        return dynamic_cast<EnumDefinition*>(ast->enum_definitions[0].get());
    }
};

TEST_F(AstEnumDefinitionTest, ValidatesImplicitEnumDefinition) {

    auto ast = parse_code("enum Direction: string { UP, DOWN }");

    ASSERT_EQ(ast->enum_definitions.size(), 1);
    auto enum_def = ast->enum_definitions[0].get();

    // 1. Verify Header
    EXPECT_EQ(enum_def->name, "Direction");
    ASSERT_NE(enum_def->underlying_type, nullptr);
    EXPECT_EQ(enum_def->underlying_type->name, "string");

    // 2. Verify Cases
    ASSERT_EQ(enum_def->cases.size(), 2);

    EXPECT_EQ(enum_def->cases[0].first, "UP");
    EXPECT_EQ(enum_def->cases[0].second, nullptr) << "Implicit case must have a null expression pointer.";

    EXPECT_EQ(enum_def->cases[1].first, "DOWN");
    EXPECT_EQ(enum_def->cases[1].second, nullptr) << "Implicit case must have a null expression pointer.";
}

TEST_F(AstEnumDefinitionTest, ValidatesMixedAndExplicitEnumDefinition) {

    auto ast = parse_code("enum StatusCode: integer { OK = 200, UNKNOWN, ERROR = 400 + 4 }");

    ASSERT_EQ(ast->enum_definitions.size(), 1);
    auto enum_def = ast->enum_definitions[0].get();

    EXPECT_EQ(enum_def->name, "StatusCode");
    EXPECT_EQ(enum_def->underlying_type->name, "integer");
    ASSERT_EQ(enum_def->cases.size(), 3);

    // Case 1: OK = 200 (Primitive Literal)
    EXPECT_EQ(enum_def->cases[0].first, "OK");
    auto ok_val = dynamic_cast<NumberLiteral*>(enum_def->cases[0].second.get());
    ASSERT_NE(ok_val, nullptr) << "Expected OK to have a NumberLiteral value.";
    EXPECT_EQ(ok_val->value, "200");

    // Case 2: UNKNOWN (Implicit)
    EXPECT_EQ(enum_def->cases[1].first, "UNKNOWN");
    EXPECT_EQ(enum_def->cases[1].second, nullptr) << "Expected UNKNOWN to be implicit (nullptr).";

    // Case 3: ERROR = 400 + 4 (Binary Expression)
    EXPECT_EQ(enum_def->cases[2].first, "ERROR");
    auto err_val = dynamic_cast<BinaryExpression*>(enum_def->cases[2].second.get());
    ASSERT_NE(err_val, nullptr) << "Expected ERROR to have a BinaryExpression value.";
    EXPECT_EQ(err_val->op, TokenType::Plus); // Or your specific plus token enum

    auto left_val = dynamic_cast<NumberLiteral*>(err_val->left.get());
    ASSERT_NE(left_val, nullptr);
    EXPECT_EQ(left_val->value, "400");

    auto right_val = dynamic_cast<NumberLiteral*>(err_val->right.get());
    ASSERT_NE(right_val, nullptr);
    EXPECT_EQ(right_val->value, "4");
}

TEST_F(AstEnumDefinitionTest, ValidatesEnumWithComplexUnderlyingType) {

    auto ast = parse_code("enum State: Result<string, Error> { SUCCESS, FAILURE }");

    ASSERT_EQ(ast->enum_definitions.size(), 1);
    auto enum_def = ast->enum_definitions[0].get();

    EXPECT_EQ(enum_def->name, "State");

    // Verify the complex type geometry
    auto type = enum_def->underlying_type.get();
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->name, "Result");
    ASSERT_EQ(type->generic_args.size(), 2);
    EXPECT_EQ(type->generic_args[0]->name, "string");
    EXPECT_EQ(type->generic_args[1]->name, "Error");

    ASSERT_EQ(enum_def->cases.size(), 2);
    EXPECT_EQ(enum_def->cases[0].first, "SUCCESS");
    EXPECT_EQ(enum_def->cases[1].first, "FAILURE");
}

TEST_F(AstEnumDefinitionTest, ValidatesEmptyEnumDefinition) {
    // Proves the parser handles immediate brace closure without crashing

    auto ast = parse_code("enum Phantom: string {}");

    ASSERT_EQ(ast->enum_definitions.size(), 1);
    auto enum_def = ast->enum_definitions[0].get();

    EXPECT_EQ(enum_def->name, "Phantom");
    EXPECT_EQ(enum_def->underlying_type->name, "string");

    // The critical assertion: the cases vector is initialized but empty
    EXPECT_TRUE(enum_def->cases.empty());
}

TEST_F(AstEnumDefinitionTest, ValidatesEnumWithDeepPostfixExpression) {
    // AST Shape of Value: BracketAccess( FunctionCall( DotAccess(sys, get_fallback) ), 0 )

    auto ast = parse_code("enum Config: string { DEFAULT = sys.get_fallback()[0] }");

    ASSERT_EQ(ast->enum_definitions.size(), 1);
    auto enum_def = ast->enum_definitions[0].get();

    ASSERT_EQ(enum_def->cases.size(), 1);
    EXPECT_EQ(enum_def->cases[0].first, "DEFAULT");

    // Root of the expression: [0]
    auto bracket_acc = dynamic_cast<BracketAccess*>(enum_def->cases[0].second.get());
    ASSERT_NE(bracket_acc, nullptr) << "Expected value to be a BracketAccess";

    auto index = dynamic_cast<NumberLiteral*>(bracket_acc->index.get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->value, "0");

    // Target of [0] -> get_fallback()
    auto func_call = dynamic_cast<FunctionCall*>(bracket_acc->target.get());
    ASSERT_NE(func_call, nullptr) << "Expected target to be a FunctionCall";

    // Target of function call -> .get_fallback
    auto dot_acc = dynamic_cast<DotAccess*>(func_call->target.get());
    ASSERT_NE(dot_acc, nullptr) << "Expected target to be a DotAccess";
    EXPECT_EQ(dot_acc->property_name, "get_fallback");

    // Target of .get_fallback -> sys
    auto sys_id = dynamic_cast<IdentifierAccess*>(dot_acc->target.get());
    ASSERT_NE(sys_id, nullptr);
    EXPECT_EQ(sys_id->name, "sys");
}

TEST_F(AstEnumDefinitionTest, ValidatesEnumUsageAsDotAccess) {
    // Proves that enum cases are seamlessly handled by the generalized DotAccess node without new parser logic.

    auto ast = parse_code("enum MyEnum: string { ONE } \n let a = my_func(a: MyEnum.ONE)");
    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);
    EXPECT_EQ(assignment->targets.size(), 1);
    EXPECT_EQ(assignment->targets[0].first, "a");

    // Verify the value is a FunctionCall
    auto func_call = dynamic_cast<FunctionCall*>(assignment->value.get());
    ASSERT_NE(func_call, nullptr);

    auto func_id = dynamic_cast<IdentifierAccess*>(func_call->target.get());
    ASSERT_NE(func_id, nullptr);
    EXPECT_EQ(func_id->name, "my_func");

    // Verify the named argument "a: MyEnum.ONE"
    ASSERT_EQ(func_call->arguments.size(), 1);
    EXPECT_EQ(func_call->arguments[0].first, "a");

    // Verify the argument value is parsed perfectly as a DotAccess
    auto dot_acc = dynamic_cast<DotAccess*>(func_call->arguments[0].second.get());
    ASSERT_NE(dot_acc, nullptr) << "Expected MyEnum.ONE to parse as a DotAccess";
    EXPECT_EQ(dot_acc->property_name, "ONE");

    // Verify the target of the dot access is "MyEnum"
    auto enum_id = dynamic_cast<IdentifierAccess*>(dot_acc->target.get());
    ASSERT_NE(enum_id, nullptr);
    EXPECT_EQ(enum_id->name, "MyEnum");
}

TEST_F(AstEnumDefinitionTest, ValidatesEnumAsTypeAnnotation) {
    // Proves the type parser treats the enum name identically to built-in types or structs.

    auto ast = parse_code("let dir: Direction = Direction.UP");
    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);
    ASSERT_EQ(assignment->targets.size(), 1);

    // Verify the Type Annotation is "Direction"
    auto type_ann = assignment->targets[0].second.get();
    ASSERT_NE(type_ann, nullptr) << "Expected a type annotation.";
    EXPECT_EQ(type_ann->name, "Direction");

    // Verify the Value is the DotAccess "Direction.UP"
    auto dot_acc = dynamic_cast<DotAccess*>(assignment->value.get());
    ASSERT_NE(dot_acc, nullptr);
    EXPECT_EQ(dot_acc->property_name, "UP");

    auto target_id = dynamic_cast<IdentifierAccess*>(dot_acc->target.get());
    ASSERT_NE(target_id, nullptr);
    EXPECT_EQ(target_id->name, "Direction");
}

TEST_F(AstEnumDefinitionTest, ValidatesEnumWithUnaryExpression) {
    // Proves unary operators work seamlessly in case values.

    auto ast = parse_code("enum Position: integer { SHORT = -1, FLAT = 0, LONG = 1 }");
    ASSERT_EQ(ast->enum_definitions.size(), 1);

    auto enum_def = ast->enum_definitions[0].get();
    ASSERT_EQ(enum_def->cases.size(), 3);

    // Check Case 0: SHORT = -1
    EXPECT_EQ(enum_def->cases[0].first, "SHORT");

    // The value should be a UnaryExpression
    auto unary_expr = dynamic_cast<UnaryExpression*>(enum_def->cases[0].second.get());
    ASSERT_NE(unary_expr, nullptr) << "Expected SHORT value to be a UnaryExpression.";
    EXPECT_EQ(unary_expr->op, TokenType::Minus);

    // The operand should be the number 1
    auto num_literal = dynamic_cast<NumberLiteral*>(unary_expr->right.get());
    ASSERT_NE(num_literal, nullptr);
    EXPECT_EQ(num_literal->value, "1");
}