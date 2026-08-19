#include <gtest/gtest.h>
#include <string>
#include "frontend/parser/helpers/parser_test_base.h"
#include "ast/ast.h"
#include "ast/ast_rewriter.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstRewriterTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstRewriterTest, ConstantFoldingBinaryExpressions)
    {
        std::string code = "let result = 1 + 2";
        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class ConstantFolder : public AstRewriter
        {
        public:
            ExprPtr rewrite_binary_expression(std::unique_ptr<BinaryExpression> bin) override
            {
                // First rewrite subtrees
                bin = ast_cast_unique<BinaryExpression>(AstRewriter::rewrite_binary_expression(std::move(bin)));
                if (!bin) return nullptr;

                // If left and right are NumberLiterals with op == Plus, fold them!
                auto* left_num = ast_cast<NumberLiteral>(bin->left.get());
                auto* right_num = ast_cast<NumberLiteral>(bin->right.get());

                if (left_num && right_num && bin->op == TokenType::Plus)
                {
                    int val = std::stoi(left_num->value) + std::stoi(right_num->value);
                    auto folded = std::make_unique<NumberLiteral>(std::to_string(val));
                    folded->span = bin->span;
                    return folded;
                }

                return bin;
            }
        };

        ConstantFolder folder;
        // Convert shared_ptr to unique_ptr for rewriting, then inspect
        // Note: For test, we can pass unique_ptr copy or rewrite_assignment directly:
        auto* assign = ast_cast<Assignment>(program->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);

        assign->value = folder.rewrite_expression(std::move(assign->value));

        auto* result_num = ast_cast<NumberLiteral>(assign->value.get());
        ASSERT_NE(result_num, nullptr);
        EXPECT_EQ(result_num->value, "3");
    }
}
