#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "frontend/parser/helpers/parser_test_base.h"
#include "ast/ast.h"
#include "ast/ast_walker.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstPruningAndControlTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstPruningAndControlTest, SkipChildrenPrunesFunctionBody)
    {
        std::string code =
            "func foo() -> int {\n"
            "    let a = 1\n"
            "    let b = 2\n"
            "    return a + b\n"
            "}\n"
            "func bar() -> int {\n"
            "    let c = 3\n"
            "    return c\n"
            "}\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class PruningWalker : public ConstAstWalker
        {
        public:
            std::vector<std::string> visited_identifiers;

            TraversalAction enter_function_definition(const FunctionDefinition& fn) override
            {
                if (fn.name.value == "foo")
                {
                    // Skip foo's body completely!
                    return TraversalAction::SkipChildren;
                }
                return TraversalAction::Continue;
            }

            TraversalAction enter_identifier_access(const IdentifierAccess& id) override
            {
                visited_identifiers.push_back(id.name.value);
                return TraversalAction::Continue;
            }
        };

        PruningWalker walker;
        walker.walk(*program);

        // Only identifiers inside bar() should be visited ('c'), none from foo()
        ASSERT_EQ(walker.visited_identifiers.size(), 1);
        EXPECT_EQ(walker.visited_identifiers[0], "c");
    }

    TEST_F(AstPruningAndControlTest, StopAbortsTraversalImmediately)
    {
        std::string code =
            "let a = 1\n"
            "let b = 2\n"
            "let c = 3\n"
            "let d = 4\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class EarlyAbortWalker : public ConstAstWalker
        {
        public:
            std::vector<std::string> seen_variables;

            TraversalAction enter_assignment(const Assignment& assign) override
            {
                if (!assign.targets.empty())
                {
                    seen_variables.push_back(assign.targets[0].name.value);
                    if (assign.targets[0].name.value == "b")
                    {
                        return TraversalAction::Stop;
                    }
                }
                return TraversalAction::Continue;
            }
        };

        EarlyAbortWalker walker;
        walker.walk(*program);

        ASSERT_EQ(walker.seen_variables.size(), 2);
        EXPECT_EQ(walker.seen_variables[0], "a");
        EXPECT_EQ(walker.seen_variables[1], "b");
    }
}
