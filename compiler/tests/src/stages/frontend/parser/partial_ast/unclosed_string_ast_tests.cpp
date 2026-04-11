#include "frontend/parser/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ImportStatementWithUnclosedString) {
        auto ast = parse_code("import \"/path/to/module", false);

        ASSERT_EQ(ast->import_statements.size(), 1);
        EXPECT_EQ(ast->import_statements[0]->path, "\"/path/to/module");
    }

    TEST_F(AstBaseTest, DirectiveWithUnclosedString) {
        auto ast = parse_code("#config \"unclosed_val", false);

        ASSERT_EQ(ast->directives.size(), 1);
        EXPECT_EQ(ast->directives[0]->name, "config");

        auto str_lit = dynamic_cast<StringLiteral *>(ast->directives[0]->value.get());
        ASSERT_NE(str_lit, nullptr);
        EXPECT_EQ(str_lit->value, "\"unclosed_val");
    }

    TEST_F(AstBaseTest, DirectiveAssignmentWithUnclosedString) {
        auto ast = parse_code("#mode = \"debug_mode", false);

        ASSERT_EQ(ast->directives.size(), 1);
        auto str_lit = dynamic_cast<StringLiteral *>(ast->directives[0]->value.get());
        ASSERT_NE(str_lit, nullptr);
        EXPECT_EQ(str_lit->value, "\"debug_mode");
    }

    TEST_F(AstBaseTest, FunctionDocstringUnclosed) {
        auto ast = parse_code("func test() -> void {\n \"\"\"This is a broken docstring\n let x = 1\n}", false);

        ASSERT_EQ(ast->function_definitions.size(), 1);
        ASSERT_TRUE(ast->function_definitions[0]->docstring.has_value());
        EXPECT_TRUE(ast->function_definitions[0]->docstring->starts_with("\"\"\"This is a broken docstring"));
    }

    TEST_F(AstBaseTest, TupleLiteralWithUnclosedString) {
        auto ast = parse_code("let t = (\"first\", \"unclosed\n)", false);

        ASSERT_FALSE(ast->execution_steps.empty());
        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto tuple = dynamic_cast<TupleLiteral *>(assignment->value.get());
        ASSERT_NE(tuple, nullptr);
        ASSERT_EQ(tuple->elements.size(), 2);

        auto str_lit = dynamic_cast<StringLiteral *>(tuple->elements[1].get());
        ASSERT_NE(str_lit, nullptr);

        EXPECT_EQ(str_lit->value, "\"unclosed");
    }

    TEST_F(AstBaseTest, DictLiteralWithUnclosedStringValue) {
        auto ast = parse_code("let d = { key: \"unclosed_val }", false);

        ASSERT_FALSE(ast->execution_steps.empty());
        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto dict = dynamic_cast<DictLiteral *>(assignment->value.get());
        ASSERT_NE(dict, nullptr);
        ASSERT_EQ(dict->elements.size(), 1);

        auto str_lit = dynamic_cast<StringLiteral *>(dict->elements[0].value.get());
        ASSERT_NE(str_lit, nullptr);
        EXPECT_EQ(str_lit->value, "\"unclosed_val }");
    }

    TEST_F(AstBaseTest, TensorLiteralWithUnclosedString) {
        auto ast = parse_code("let v = [\"val1\", \"val2]", false);

        ASSERT_FALSE(ast->execution_steps.empty());
        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto tensor = dynamic_cast<TensorLiteral *>(assignment->value.get());
        ASSERT_NE(tensor, nullptr);
        ASSERT_EQ(tensor->elements.size(), 2);

        auto str_lit = dynamic_cast<StringLiteral *>(tensor->elements[1].get());
        ASSERT_NE(str_lit, nullptr);
        EXPECT_EQ(str_lit->value, "\"val2]");
    }

    TEST_F(AstBaseTest, EnumCaseValueWithUnclosedString) {
        auto ast = parse_code("enum Status: string { Error = \"failure }", false);

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        ASSERT_EQ(ast->enum_definitions[0]->cases.size(), 1);

        auto str_lit = dynamic_cast<StringLiteral *>(ast->enum_definitions[0]->cases[0].value.get());
        ASSERT_NE(str_lit, nullptr);
        EXPECT_EQ(str_lit->value, "\"failure }");
    }

    TEST_F(AstBaseTest, ModifierArgumentWithUnclosedString) {
        auto ast = parse_code("@Deprecated(reason: \"not_safe)\nlet x = 1", false);

        ASSERT_FALSE(ast->execution_steps.empty());
        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);
        ASSERT_EQ(assignment->modifiers.size(), 1);

        auto &mod = assignment->modifiers[0];
        ASSERT_EQ(mod.arguments.size(), 1);
        EXPECT_EQ(mod.arguments[0].first, "reason");

        auto str_lit = dynamic_cast<StringLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(str_lit, nullptr);
        EXPECT_EQ(str_lit->value, "\"not_safe)");
    }
}
