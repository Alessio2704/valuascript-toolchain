#include <gtest/gtest.h>
#include <string>
#include "frontend/parser/helpers/parser_test_base.h"
#include "ast/ast.h"
#include "ast/ast_walker.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstAncestorStackTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstAncestorStackTest, ContextQueriesReflectNodeHierarchy)
    {
        std::string code =
            "func calculate(x: int) -> int {\n"
            "    return x + 10\n"
            "}\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class ContextInspector : public ConstAstWalker
        {
        public:
            bool checked_number = false;

            TraversalAction enter_number_literal(const NumberLiteral& num) override
            {
                if (num.value == "10")
                {
                    checked_number = true;

                    // 1. Parent should be BinaryExpression
                    const AstNode* p = parent();
                    EXPECT_NE(p, nullptr);
                    if (p) EXPECT_EQ(p->kind, AstKind::BinaryExpression);

                    // 2. Depth should be > 2
                    EXPECT_GT(depth(), 2);

                    // 3. Enclosing function should be calculate
                    const FunctionDefinition* fn = enclosing_function();
                    EXPECT_NE(fn, nullptr);
                    if (fn) EXPECT_EQ(fn->name.value, "calculate");

                    // 4. Enclosing statement should be ReturnStatement
                    const Statement* stmt = enclosing_statement();
                    EXPECT_NE(stmt, nullptr);
                    if (stmt) EXPECT_EQ(stmt->kind, AstKind::ReturnStatement);

                    // 5. find_ancestor<Program>() should find root
                    const Program* prog = find_ancestor<Program>();
                    EXPECT_NE(prog, nullptr);
                }
                return TraversalAction::Continue;
            }
        };

        ContextInspector inspector;
        inspector.walk(*program);

        EXPECT_TRUE(inspector.checked_number);
    }

    TEST_F(AstAncestorStackTest, EnclosingScopeHelpersLocateAllContainers)
    {
        std::string code =
            "extension Point {\n"
            "    struct Inner {\n"
            "        val: int\n"
            "    }\n"
            "    enum Status: int {\n"
            "        Active = 1\n"
            "    }\n"
            "    func get_status(code: int) -> int {\n"
            "        let res = switch (code) {\n"
            "            case Active -> 100\n"
            "            default -> 0\n"
            "        }\n"
            "        return res\n"
            "    }\n"
            "}\n";

        auto program = parse_code(code);
        ASSERT_NE(program, nullptr);

        class DeepScopeInspector : public ConstAstWalker
        {
        public:
            bool checked_struct_field = false;
            bool checked_enum_case = false;
            bool checked_switch_result = false;

            TraversalAction visit_struct_field(const StructField& f) override
            {
                if (f.name.value == "val")
                {
                    checked_struct_field = true;
                    const StructDefinition* st = enclosing_struct();
                    EXPECT_NE(st, nullptr);
                    if (st) EXPECT_EQ(st->name.value, "Inner");

                    const ExtensionDefinition* ext = enclosing_extension();
                    EXPECT_NE(ext, nullptr);
                    if (ext && ext->target_type) EXPECT_EQ(ext->target_type->name.value, "Point");
                }
                return TraversalAction::Continue;
            }

            TraversalAction visit_enum_case(const valuascript::compiler::EnumCase& c) override
            {
                if (c.name.value == "Active")
                {
                    checked_enum_case = true;
                    const EnumDefinition* en = enclosing_enum();
                    EXPECT_NE(en, nullptr);
                    if (en) EXPECT_EQ(en->name.value, "Status");

                    const ExtensionDefinition* ext = enclosing_extension();
                    EXPECT_NE(ext, nullptr);
                    if (ext && ext->target_type) EXPECT_EQ(ext->target_type->name.value, "Point");
                }
                return TraversalAction::Continue;
            }

            TraversalAction enter_number_literal(const NumberLiteral& n) override
            {
                if (n.value == "100")
                {
                    checked_switch_result = true;
                    const SwitchExpression* sw = enclosing_switch();
                    EXPECT_NE(sw, nullptr);

                    const FunctionDefinition* fn = enclosing_function();
                    EXPECT_NE(fn, nullptr);
                    if (fn) EXPECT_EQ(fn->name.value, "get_status");

                    const ExtensionDefinition* ext = enclosing_extension();
                    EXPECT_NE(ext, nullptr);
                    if (ext && ext->target_type) EXPECT_EQ(ext->target_type->name.value, "Point");

                    const Program* prog = enclosing_program();
                    EXPECT_NE(prog, nullptr);
                }
                return TraversalAction::Continue;
            }
        };

        DeepScopeInspector inspector;
        inspector.walk(*program);

        EXPECT_TRUE(inspector.checked_struct_field);
        EXPECT_TRUE(inspector.checked_enum_case);
        EXPECT_TRUE(inspector.checked_switch_result);
    }
}
