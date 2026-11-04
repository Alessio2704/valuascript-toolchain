#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>

#include "ast_walker_test_helper.h"

namespace valuascript::compiler::test
{
    class ProgramHierarchyStatsWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        size_t total_enter_nodes = 0;
        size_t total_leave_nodes = 0;
        size_t programs = 0;
        size_t imports = 0;
        size_t directives = 0;
        size_t functions = 0;
        size_t structs = 0;
        size_t enums = 0;
        size_t aliases = 0;
        size_t extensions = 0;
        size_t assignments = 0;
        size_t reassignments = 0;
        size_t expr_stmts = 0;
        size_t returns = 0;
        size_t expressions = 0;
        size_t type_annotations = 0;
        size_t modifiers = 0;
        size_t params = 0;
        size_t fields = 0;
        size_t cases = 0;
        size_t call_args = 0;
        size_t comments = 0;
        size_t max_depth = 0;

        TraversalAction enter_node(AstNode&) override
        {
            total_enter_nodes++;
            max_depth = std::max(max_depth, depth());
            return TraversalAction::Continue;
        }

        void leave_node(AstNode&) override
        {
            total_leave_nodes++;
        }

        TraversalAction enter(Program&) override { programs++; return TraversalAction::Continue; }
        TraversalAction enter(ImportStatement&) override { imports++; return TraversalAction::Continue; }
        TraversalAction enter(Directive&) override { directives++; return TraversalAction::Continue; }
        TraversalAction enter(FunctionDefinition&) override { functions++; return TraversalAction::Continue; }
        TraversalAction enter(StructDefinition&) override { structs++; return TraversalAction::Continue; }
        TraversalAction enter(EnumDefinition&) override { enums++; return TraversalAction::Continue; }
        TraversalAction enter(TypeAliasDefinition&) override { aliases++; return TraversalAction::Continue; }
        TraversalAction enter(ExtensionDefinition&) override { extensions++; return TraversalAction::Continue; }
        TraversalAction enter(Assignment&) override { assignments++; return TraversalAction::Continue; }
        TraversalAction enter(Reassignment&) override { reassignments++; return TraversalAction::Continue; }
        TraversalAction enter(ExpressionStatement&) override { expr_stmts++; return TraversalAction::Continue; }
        TraversalAction enter(ReturnStatement&) override { returns++; return TraversalAction::Continue; }
        TraversalAction enter(Expression&) override { expressions++; return TraversalAction::Continue; }
        TraversalAction enter(TypeAnnotation&) override { type_annotations++; return TraversalAction::Continue; }
        TraversalAction enter(Modifier&) override { modifiers++; return TraversalAction::Continue; }
        TraversalAction enter(FunctionParameter&) override { params++; return TraversalAction::Continue; }
        TraversalAction enter(StructField&) override { fields++; return TraversalAction::Continue; }
        TraversalAction enter(EnumCase&) override { cases++; return TraversalAction::Continue; }
        TraversalAction enter(CallArgument&) override { call_args++; return TraversalAction::Continue; }
        TraversalAction enter(Comment&) override { comments++; return TraversalAction::Continue; }
    };

    TEST(AstWalkerProgramTest, CompleteProgramTraversalVisitsAllHierarchies)
    {
        auto prog = create_sample_program(2);
        ASSERT_NE(prog, nullptr);

        prog->execution_steps.push_back(create_sample<Assignment>(2));
        prog->execution_steps.push_back(create_sample<Reassignment>(2));
        prog->execution_steps.push_back(create_sample<ReturnStatement>(2));

        ProgramHierarchyStatsWalker walker;
        walker.walk(*prog);

        EXPECT_EQ(walker.programs, 1);
        EXPECT_GT(walker.imports, 0);
        EXPECT_GT(walker.directives, 0);
        EXPECT_GT(walker.functions, 0);
        EXPECT_GT(walker.structs, 0);
        EXPECT_GT(walker.enums, 0);
        EXPECT_GT(walker.aliases, 0);
        EXPECT_GT(walker.extensions, 0);
        EXPECT_GT(walker.assignments, 0);
        EXPECT_GT(walker.reassignments, 0);
        EXPECT_GT(walker.expr_stmts, 0);
        EXPECT_GT(walker.returns, 0);
        EXPECT_GT(walker.expressions, 0);
        EXPECT_GT(walker.type_annotations, 0);
        EXPECT_GT(walker.modifiers, 0);
        EXPECT_GT(walker.params, 0);
        EXPECT_GT(walker.fields, 0);
        EXPECT_GT(walker.cases, 0);
        EXPECT_GT(walker.call_args, 0);
        EXPECT_GT(walker.comments, 0);

        EXPECT_GT(walker.max_depth, 1);
        EXPECT_EQ(walker.total_enter_nodes, walker.total_leave_nodes);
        EXPECT_EQ(walker.depth(), 0);
    }
}
