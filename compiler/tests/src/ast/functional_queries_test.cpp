#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "frontend/parser/helpers/parser_test_base.h"
#include "ast/ast.h"
#include "ast/ast_query.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstFunctionalQueriesTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstFunctionalQueriesTest, ForEachNodeFiltersByType)
    {
        std::string code =
            "func foo() -> int { return 1 }\n"
            "func bar() -> int { return 2 }\n"
            "struct Point { x: int }\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        std::vector<std::string> func_names;
        for_each_node<FunctionDefinition>(*program, [&func_names](const FunctionDefinition& fn) {
            func_names.push_back(fn.name.value);
        });

        ASSERT_EQ(func_names.size(), 2);
        EXPECT_EQ(func_names[0], "foo");
        EXPECT_EQ(func_names[1], "bar");
    }

    TEST_F(AstFunctionalQueriesTest, CollectNodesAccumulatesMatches)
    {
        std::string code = "let arr = [10, 20, 30]";
        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        std::vector<const NumberLiteral*> numbers = collect_nodes<NumberLiteral>(*program);

        ASSERT_EQ(numbers.size(), 3);
        EXPECT_EQ(numbers[0]->value, "10");
        EXPECT_EQ(numbers[1]->value, "20");
        EXPECT_EQ(numbers[2]->value, "30");
    }

    TEST_F(AstFunctionalQueriesTest, FindNodeAtPositionLocatesInnermostNode)
    {
        std::string code =
            "func add(a: int, b: int) -> int {\n"
            "    return a + b\n"
            "}\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        // Position: line 2, column 12 corresponds to 'a' in 'return a + b'
        const AstNode* node = find_node_at_position(*program, 2, 12);
        ASSERT_NE(node, nullptr);
        EXPECT_EQ(node->kind, AstKind::IdentifierAccess);

        auto* id = ast_cast<const IdentifierAccess>(node);
        ASSERT_NE(id, nullptr);
        EXPECT_EQ(id->name.value, "a");
    }

    TEST_F(AstFunctionalQueriesTest, FindAncestorPathAtPositionReturnsFullAncestry)
    {
        std::string code =
            "func add(a: int, b: int) -> int {\n"
            "    return a + b\n"
            "}\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        // Position: line 2, column 12 ('a')
        std::vector<const AstNode*> path = find_ancestor_path_at_position(*program, 2, 12);

        ASSERT_GE(path.size(), 4);
        EXPECT_EQ(path[0]->kind, AstKind::Program);
        EXPECT_EQ(path[1]->kind, AstKind::FunctionDefinition);
        EXPECT_EQ(path[2]->kind, AstKind::ReturnStatement);
        EXPECT_EQ(path[3]->kind, AstKind::BinaryExpression);
        EXPECT_EQ(path.back()->kind, AstKind::IdentifierAccess);
    }
}
