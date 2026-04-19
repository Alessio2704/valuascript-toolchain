#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test
{
    TEST_F(AstBaseTest, ValidatesExpressionStatementGeometry)
    {
        auto ast = parse_code("sys.init()");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto expr_stmt = dynamic_cast<ExpressionStatement*>(ast->execution_steps[0].get());
        ASSERT_NE(expr_stmt, nullptr) << "Expected an ExpressionStatement node.";

        auto func_call = dynamic_cast<FunctionCall*>(expr_stmt->expr.get());
        ASSERT_NE(func_call, nullptr) << "Expected the wrapped expression to be a FunctionCall.";

        auto dot_access = dynamic_cast<DotAccess*>(func_call->target.get());
        ASSERT_NE(dot_access, nullptr);
        EXPECT_EQ(dot_access->property_name, "init");

        auto target_id = dynamic_cast<IdentifierAccess*>(dot_access->target.get());
        ASSERT_NE(target_id, nullptr);
        EXPECT_EQ(target_id->name, "sys");
    }
}
