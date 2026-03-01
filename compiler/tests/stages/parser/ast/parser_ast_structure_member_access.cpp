#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/lexer/lexer_stage.h"
#include "stages/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstMemberAccessTest : public testing::Test {
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

    Assignment* get_first_assignment(const std::shared_ptr<Program>& ast) {
        if (ast->execution_steps.empty()) return nullptr;
        return dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    }
};

TEST_F(AstMemberAccessTest, ValidatesDeepMemberAccessLeftAssociativity) {

    auto ast = parse_code("let value = company.department.manager");
    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    // 1. Root: .manager
    auto root_access = dynamic_cast<MemberAccess*>(assignment->value.get());
    ASSERT_NE(root_access, nullptr) << "Root expression must be MemberAccess for 'manager'";
    EXPECT_EQ(root_access->property_name, "manager");

    // 2. Target of .manager -> .department
    auto dept_access = dynamic_cast<MemberAccess*>(root_access->target.get());
    ASSERT_NE(dept_access, nullptr) << "Target must be MemberAccess for 'department'";
    EXPECT_EQ(dept_access->property_name, "department");

    // 3. Target of .department -> company
    auto company_id = dynamic_cast<IdentifierAccess*>(dept_access->target.get());
    ASSERT_NE(company_id, nullptr) << "Target must be IdentifierAccess for 'company'";
    EXPECT_EQ(company_id->name, "company");
}

TEST_F(AstMemberAccessTest, ValidatesMixedPostfixChaining) {
    // AST Shape: MemberAccess( TensorAccess( FunctionCall( MemberAccess(api, get_data), 1 ), 0 ), value )

    auto ast = parse_code("let result = api.get_data(id: 1)[0].value");
    ASSERT_EQ(ast->execution_steps.size(), 1);
    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());

    // 1. Root: .value
    auto root_access = dynamic_cast<MemberAccess*>(assignment->value.get());
    ASSERT_NE(root_access, nullptr);
    EXPECT_EQ(root_access->property_name, "value");

    // 2. Target of .value -> [0]
    auto tensor_access = dynamic_cast<TensorAccess*>(root_access->target.get());
    ASSERT_NE(tensor_access, nullptr);
    auto index = dynamic_cast<NumberLiteral*>(tensor_access->index.get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->value, "0");

    // 3. Target of [0] -> get_data(id: 1)
    auto func_call = dynamic_cast<FunctionCall*>(tensor_access->target.get());
    ASSERT_NE(func_call, nullptr);
    ASSERT_EQ(func_call->arguments.size(), 1);
    EXPECT_EQ(func_call->arguments[0].first, "id");

    // 4. Target of function call -> .get_data
    auto get_data_access = dynamic_cast<MemberAccess*>(func_call->target.get());
    ASSERT_NE(get_data_access, nullptr);
    EXPECT_EQ(get_data_access->property_name, "get_data");

    // 5. Target of .get_data -> api
    auto api_id = dynamic_cast<IdentifierAccess*>(get_data_access->target.get());
    ASSERT_NE(api_id, nullptr);
    EXPECT_EQ(api_id->name, "api");
}

TEST_F(AstMemberAccessTest, ValidatesMemberAccessPrecedenceOverMath) {
    // AST Shape: BinaryExpression( *, MemberAccess(box, width), MemberAccess(box, height) )

    auto ast = parse_code("let area = box.width * box.height");
    ASSERT_EQ(ast->execution_steps.size(), 1);
    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());

    // 1. Root: * (Multiplication)
    auto root_math = dynamic_cast<BinaryExpression*>(assignment->value.get());
    ASSERT_NE(root_math, nullptr) << "Root expression must be a BinaryExpression";
    EXPECT_EQ(root_math->op, TokenType::Star); // Or whatever your enum uses

    // 2. Left side -> box.width
    auto left_access = dynamic_cast<MemberAccess*>(root_math->left.get());
    ASSERT_NE(left_access, nullptr);
    EXPECT_EQ(left_access->property_name, "width");

    auto left_target = dynamic_cast<IdentifierAccess*>(left_access->target.get());
    ASSERT_NE(left_target, nullptr);
    EXPECT_EQ(left_target->name, "box");

    // 3. Right side -> box.height
    auto right_access = dynamic_cast<MemberAccess*>(root_math->right.get());
    ASSERT_NE(right_access, nullptr);
    EXPECT_EQ(right_access->property_name, "height");

    auto right_target = dynamic_cast<IdentifierAccess*>(right_access->target.get());
    ASSERT_NE(right_target, nullptr);
    EXPECT_EQ(right_target->name, "box");
}