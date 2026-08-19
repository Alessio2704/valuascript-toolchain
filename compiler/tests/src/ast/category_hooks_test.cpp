#include <gtest/gtest.h>
#include <string>
#include "frontend/parser/helpers/parser_test_base.h"
#include "ast/ast.h"
#include "ast/ast_walker.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstCategoryHooksTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstCategoryHooksTest, InterceptsAllExpressionsUniversally)
    {
        std::string code = "func test() -> bool {\nlet total = (1 + 2) * 3\nreturn total > 0\n}\n";
        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class ExpressionCounter : public ConstAstWalker
        {
        public:
            size_t enter_count = 0;
            size_t leave_count = 0;

            TraversalAction enter_expression(const Expression&) override
            {
                ++enter_count;
                return TraversalAction::Continue;
            }

            void leave_expression(const Expression&) override
            {
                ++leave_count;
            }
        };

        ExpressionCounter counter;
        counter.walk(*program);

        // Expressions present:
        // (1 + 2) * 3 [BinaryExpression]
        // (1 + 2)     [GroupingExpression]
        // 1 + 2       [BinaryExpression]
        // 1           [NumberLiteral]
        // 2           [NumberLiteral]
        // 3           [NumberLiteral]
        // total > 0   [BinaryExpression]
        // total       [IdentifierAccess]
        // 0           [NumberLiteral]
        EXPECT_EQ(counter.enter_count, 9);
        EXPECT_EQ(counter.leave_count, 9);
    }

    TEST_F(AstCategoryHooksTest, InterceptsAllStatementsAndDeclarations)
    {
        std::string code =
            "struct Point { x: int }\n"
            "func get_x() -> int { return 10 }\n"
            "let a = 1\n"
            "a = 2\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class CategoryCounter : public ConstAstWalker
        {
        public:
            size_t decl_count = 0;
            size_t stmt_count = 0;

            TraversalAction enter_declaration(const AstNode&) override
            {
                ++decl_count;
                return TraversalAction::Continue;
            }

            TraversalAction enter_statement(const Statement&) override
            {
                ++stmt_count;
                return TraversalAction::Continue;
            }
        };

        CategoryCounter counter;
        counter.walk(*program);

        // Declarations: struct Point, func get_x
        EXPECT_EQ(counter.decl_count, 2);

        // Statements: return 10 (inside func), let a = 1, a = 2
        EXPECT_EQ(counter.stmt_count, 3);
    }
}
