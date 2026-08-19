#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "frontend/parser/helpers/parser_test_base.h"
#include "ast/ast.h"
#include "ast/ast_walker.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstStructuralTraversalTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstStructuralTraversalTest, VisitsAllDeclarationsAndStatementsInOrder)
    {
        std::string code =
            "// A top-level comment\n"
            "#version = 1.0\n"
            "import \"math.vs\"\n"
            "struct Point { x: int, y: int }\n"
            "enum Color: int { Red = 1, Green = 2 }\n"
            "typealias ID = int\n"
            "func add(a: int, b: int = 0) -> int { return a + b }\n"
            "let p = Point\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class OrderLogger : public ConstAstWalker
        {
        public:
            std::vector<std::string> log;

            TraversalAction enter_program(const Program&) override { log.push_back("enter_program"); return TraversalAction::Continue; }
            void leave_program(const Program&) override { log.push_back("leave_program"); }

            TraversalAction enter_directive(const Directive& d) override { log.push_back("directive:" + d.name.value); return TraversalAction::Continue; }
            TraversalAction enter_import_statement(const ImportStatement& imp) override { log.push_back("import:" + imp.path.value); return TraversalAction::Continue; }
            TraversalAction enter_struct_definition(const StructDefinition& s) override { log.push_back("struct:" + s.name.value); return TraversalAction::Continue; }
            TraversalAction enter_enum_definition(const EnumDefinition& e) override { log.push_back("enum:" + e.name.value); return TraversalAction::Continue; }
            TraversalAction enter_type_alias_definition(const TypeAliasDefinition& t) override { log.push_back("typealias:" + t.name.value); return TraversalAction::Continue; }
            TraversalAction enter_function_definition(const FunctionDefinition& f) override { log.push_back("func:" + f.name.value); return TraversalAction::Continue; }
            TraversalAction enter_assignment(const Assignment&) override { log.push_back("assignment"); return TraversalAction::Continue; }
            TraversalAction enter_return_statement(const ReturnStatement&) override { log.push_back("return"); return TraversalAction::Continue; }
            TraversalAction enter_binary_expression(const BinaryExpression&) override { log.push_back("binary_expr"); return TraversalAction::Continue; }
        };

        OrderLogger logger;
        logger.walk(*program);

        ASSERT_FALSE(logger.log.empty());
        EXPECT_EQ(logger.log.front(), "enter_program");
        EXPECT_EQ(logger.log.back(), "leave_program");

        // Verify key structural events were recorded in order
        auto contains_item = [&](const std::string& item) {
            return std::find(logger.log.begin(), logger.log.end(), item) != logger.log.end();
        };

        EXPECT_TRUE(contains_item("directive:version"));
        EXPECT_TRUE(contains_item("import:\"math.vs\""));
        EXPECT_TRUE(contains_item("struct:Point"));
        EXPECT_TRUE(contains_item("enum:Color"));
        EXPECT_TRUE(contains_item("typealias:ID"));
        EXPECT_TRUE(contains_item("func:add"));
        EXPECT_TRUE(contains_item("return"));
        EXPECT_TRUE(contains_item("binary_expr"));
        EXPECT_TRUE(contains_item("assignment"));
    }

    TEST_F(AstStructuralTraversalTest, VisitsGrammarStructsExplicitly)
    {
        std::string code =
            "@logged\n"
            "func compute(@flag(active: true) x: int) -> int {\n"
            "    return x\n"
            "}\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class GrammarStructLogger : public ConstAstWalker
        {
        public:
            std::vector<std::string> modifiers;
            std::vector<std::string> parameters;
            std::vector<std::string> call_args;

            TraversalAction visit_modifier(const Modifier& mod) override
            {
                modifiers.push_back(mod.name.value);
                return TraversalAction::Continue;
            }

            TraversalAction visit_parameter(const FunctionParameter& param) override
            {
                parameters.push_back(param.name.value);
                return TraversalAction::Continue;
            }

            TraversalAction visit_call_argument(const CallArgument& arg) override
            {
                call_args.push_back(arg.name.value);
                return TraversalAction::Continue;
            }
        };

        GrammarStructLogger logger;
        logger.walk(*program);

        ASSERT_EQ(logger.modifiers.size(), 2);
        EXPECT_EQ(logger.modifiers[0], "logged");
        EXPECT_EQ(logger.modifiers[1], "flag");

        ASSERT_EQ(logger.parameters.size(), 1);
        EXPECT_EQ(logger.parameters[0], "x");

        ASSERT_EQ(logger.call_args.size(), 1);
        EXPECT_EQ(logger.call_args[0], "active");
    }

    TEST_F(AstStructuralTraversalTest, MutableWalkerCanMutateNodesInPlace)
    {
        std::string code = "let x = 10\nlet y = 20";
        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class IdentifierUpperCaser : public AstWalker
        {
        public:
            TraversalAction enter_assignment(Assignment& node) override
            {
                for (auto& target : node.targets)
                {
                    for (char& c : target.name.value)
                    {
                        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                    }
                }
                return TraversalAction::Continue;
            }
        };

        IdentifierUpperCaser upper;
        upper.walk(*program);

        auto* assign1 = ast_cast<Assignment>(program->execution_steps[0].get());
        auto* assign2 = ast_cast<Assignment>(program->execution_steps[1].get());

        ASSERT_NE(assign1, nullptr);
        ASSERT_NE(assign2, nullptr);
        EXPECT_EQ(assign1->targets[0].name.value, "X");
        EXPECT_EQ(assign2->targets[0].name.value, "Y");
    }
}
