#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstNameSpanTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }

        static void assert_span(const SourceSpan& span,
                                size_t line_start, size_t col_start,
                                size_t line_end, size_t col_end,
                                size_t start_offset, size_t length)
        {
            EXPECT_EQ(span.line_start, line_start) << "Mismatch in line_start";
            EXPECT_EQ(span.column_start, col_start) << "Mismatch in column_start";
            EXPECT_EQ(span.line_end, line_end) << "Mismatch in line_end";
            EXPECT_EQ(span.column_end, col_end) << "Mismatch in column_end";
            EXPECT_EQ(span.start_offset, start_offset) << "Mismatch in start_offset";
            EXPECT_EQ(span.length, length) << "Mismatch in length";
            EXPECT_EQ(span.end_offset(), start_offset + length) << "Mismatch in end_offset()";
        }
    };

    TEST_F(AstNameSpanTest, FunctionAndParameterNameSpans)
    {
        std::string code = "func calculateSum(firstVal: int, @attr secondVal: int = 10) -> int {\n"
                           "    return firstVal + secondVal\n"
                           "}";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->function_definitions.size(), 1);
        const auto& func = ast->function_definitions[0];

        // "calculateSum": line 1, col 6 -> col 18, offset 5, len 12
        EXPECT_EQ(func->name, "calculateSum");
        assert_span(func->name.span, 1, 6, 1, 18, 5, 12);

        ASSERT_EQ(func->parameters.size(), 2);
        // Param 1: "firstVal"
        // line 1, col 19 -> col 27, offset 18, len 8
        EXPECT_EQ(func->parameters[0].name, "firstVal");
        assert_span(func->parameters[0].name.span, 1, 19, 1, 27, 18, 8);
        // Full param 1 span: "firstVal: int" -> line 1, col 19 -> 32, offset 18, len 13
        assert_span(func->parameters[0].span, 1, 19, 1, 32, 18, 13);

        // Param 2: "secondVal"
        // "@attr " is 6 chars starting at col 34 (offset 33).
        // "secondVal" starts at col 40 (offset 39), len 9 -> col 49
        EXPECT_EQ(func->parameters[1].name, "secondVal");
        assert_span(func->parameters[1].name.span, 1, 40, 1, 49, 39, 9);
        // Full param 2 span starts at "@" (col 34, offset 33) through "10" (col 59, offset 58, len 2 -> ends at col 59, byte 58)
        assert_span(func->parameters[1].span, 1, 34, 1, 59, 33, 25);
    }

    TEST_F(AstNameSpanTest, StructAndFieldNameSpans)
    {
        std::string code = "struct UserProfile {\n"
                           "    id: int,\n"
                           "    @attr displayName: string\n"
                           "}";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->struct_definitions.size(), 1);
        const auto& st = ast->struct_definitions[0];

        // "UserProfile": line 1, col 8 -> col 19, offset 7, len 11
        EXPECT_EQ(st->name, "UserProfile");
        assert_span(st->name.span, 1, 8, 1, 19, 7, 11);

        ASSERT_EQ(st->fields.size(), 2);
        // Field 1: "id: int" on line 2
        // "    id" starts at col 5 (offset 25), len 2 -> col 7
        EXPECT_EQ(st->fields[0].name, "id");
        assert_span(st->fields[0].name.span, 2, 5, 2, 7, 25, 2);
        assert_span(st->fields[0].span, 2, 5, 2, 12, 25, 7);

        // Field 2: "@attr displayName: string" on line 3
        // "    @attr displayName" -> "@attr" starts at col 5 (offset 38). "displayName" starts at col 11 (offset 44), len 11 -> col 22
        EXPECT_EQ(st->fields[1].name, "displayName");
        assert_span(st->fields[1].name.span, 3, 11, 3, 22, 44, 11);
        assert_span(st->fields[1].span, 3, 5, 3, 30, 38, 25);
    }

    TEST_F(AstNameSpanTest, EnumAndCaseNameSpans)
    {
        std::string code = "enum Status: int {\n"
                           "    Active = 1,\n"
                           "    Inactive = 2\n"
                           "}";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->enum_definitions.size(), 1);
        const auto& en = ast->enum_definitions[0];

        // "Status": line 1, col 6 -> col 12, offset 5, len 6
        EXPECT_EQ(en->name, "Status");
        assert_span(en->name.span, 1, 6, 1, 12, 5, 6);

        ASSERT_EQ(en->cases.size(), 2);
        // Case 1: "Active = 1" on line 2
        // "    Active" starts at col 5 (offset 23), len 6 -> col 11
        EXPECT_EQ(en->cases[0].name, "Active");
        assert_span(en->cases[0].name.span, 2, 5, 2, 11, 23, 6);
        assert_span(en->cases[0].span, 2, 5, 2, 15, 23, 10);

        // Case 2: "Inactive = 2" on line 3
        // "    Inactive" starts at col 5 (offset 39), len 8 -> col 13
        EXPECT_EQ(en->cases[1].name, "Inactive");
        assert_span(en->cases[1].name.span, 3, 5, 3, 13, 39, 8);
        assert_span(en->cases[1].span, 3, 5, 3, 17, 39, 12);
    }

    TEST_F(AstNameSpanTest, TypeAliasNameSpan)
    {
        std::string code = "typealias UserID = int";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->type_aliases.size(), 1);
        const auto& alias = ast->type_aliases[0];

        // "UserID": line 1, col 11 -> col 17, offset 10, len 6
        EXPECT_EQ(alias->name, "UserID");
        assert_span(alias->name.span, 1, 11, 1, 17, 10, 6);
    }

    TEST_F(AstNameSpanTest, DirectiveAndImportSpans)
    {
        std::string code = "#compiler_opt = true\n"
                           "import \"std/math\"";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->directives.size(), 1);
        ASSERT_EQ(ast->import_statements.size(), 1);

        const auto& dir = ast->directives[0];
        // "#compiler_opt" -> name is "compiler_opt", starts at col 2 (offset 1), len 12 -> col 14
        EXPECT_EQ(dir->name, "compiler_opt");
        assert_span(dir->name.span, 1, 2, 1, 14, 1, 12);

        const auto& imp = ast->import_statements[0];
        // "import \"std/math\"" on line 2
        // "\"std/math\"" starts at col 8 (offset 28), len 10 -> col 18
        EXPECT_EQ(imp->path, "\"std/math\"");
        assert_span(imp->path.span, 2, 8, 2, 18, 28, 10);
    }

    TEST_F(AstNameSpanTest, AssignmentTargetNameSpans)
    {
        std::string code = "let firstVar: int, @attr secondVar = 100";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);
        ASSERT_EQ(assign->targets.size(), 2);

        // Target 1: "firstVar: int"
        // "let firstVar" -> starts at col 5 (offset 4), len 8 -> col 13
        EXPECT_EQ(assign->targets[0].name, "firstVar");
        assert_span(assign->targets[0].name.span, 1, 5, 1, 13, 4, 8);
        assert_span(assign->targets[0].span, 1, 5, 1, 18, 4, 13);

        // Target 2: "@attr secondVar"
        // "@attr" starts at col 20 (offset 19). "secondVar" starts at col 26 (offset 25), len 9 -> col 35
        EXPECT_EQ(assign->targets[1].name, "secondVar");
        assert_span(assign->targets[1].name.span, 1, 26, 1, 35, 25, 9);
        assert_span(assign->targets[1].span, 1, 20, 1, 35, 19, 15);
    }

    TEST_F(AstNameSpanTest, DotAccessPropertySpans)
    {
        std::string code = "let val = user.profile.address";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);

        // Outer dot access: `.address`
        auto outer_dot = dynamic_cast<DotAccess*>(assign->value.get());
        ASSERT_NE(outer_dot, nullptr);
        EXPECT_EQ(outer_dot->property_name, "address");
        // "address": line 1, col 24 (offset 23), len 7 -> col 31
        assert_span(outer_dot->property_name.span, 1, 24, 1, 31, 23, 7);

        // Inner dot access: `.profile`
        auto inner_dot = dynamic_cast<DotAccess*>(outer_dot->target.get());
        ASSERT_NE(inner_dot, nullptr);
        EXPECT_EQ(inner_dot->property_name, "profile");
        // "profile": line 1, col 16 (offset 15), len 7 -> col 23
        assert_span(inner_dot->property_name.span, 1, 16, 1, 23, 15, 7);
    }

    TEST_F(AstNameSpanTest, DictItemKeySpans)
    {
        std::string code = "let d = { alpha: 1, @attr beta: 2 }";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);

        auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
        ASSERT_NE(dict, nullptr);
        ASSERT_EQ(dict->elements.size(), 2);

        // Item 1: "alpha: 1"
        // "alpha" starts at col 11 (offset 10), len 5 -> col 16
        EXPECT_EQ(dict->elements[0].key, "alpha");
        assert_span(dict->elements[0].key.span, 1, 11, 1, 16, 10, 5);
        assert_span(dict->elements[0].span, 1, 11, 1, 19, 10, 8);

        // Item 2: "@attr beta: 2"
        // "@attr" starts at col 21 (offset 20). "beta" starts at col 27 (offset 26), len 4 -> col 31
        EXPECT_EQ(dict->elements[1].key, "beta");
        assert_span(dict->elements[1].key.span, 1, 27, 1, 31, 26, 4);
        assert_span(dict->elements[1].span, 1, 21, 1, 34, 20, 13);
    }

    TEST_F(AstNameSpanTest, SwitchCaseIdentifierSpans)
    {
        std::string code = "let res = switch (x) {\n"
                           "    case Alpha, Beta, Gamma -> 10\n"
                           "    default -> 0\n"
                           "}";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);

        auto sw = dynamic_cast<SwitchExpression*>(assign->value.get());
        ASSERT_NE(sw, nullptr);
        ASSERT_EQ(sw->cases.size(), 1);

        const auto& c = sw->cases[0];
        ASSERT_EQ(c.identifiers.size(), 3);

        // Line 2: "    case Alpha, Beta, Gamma -> 10"
        // "Alpha": col 10 (offset 32), len 5 -> col 15
        EXPECT_EQ(c.identifiers[0], "Alpha");
        assert_span(c.identifiers[0].span, 2, 10, 2, 15, 32, 5);
        // "Beta": col 17 (offset 39), len 4 -> col 21
        EXPECT_EQ(c.identifiers[1], "Beta");
        assert_span(c.identifiers[1].span, 2, 17, 2, 21, 39, 4);
        // "Gamma": col 23 (offset 45), len 5 -> col 28
        EXPECT_EQ(c.identifiers[2], "Gamma");
        assert_span(c.identifiers[2].span, 2, 23, 2, 28, 45, 5);
        // Full case branch span: "case Alpha, Beta, Gamma -> 10" -> col 5 -> 34, offset 27, len 29
        assert_span(c.span, 2, 5, 2, 34, 27, 29);
    }

    TEST_F(AstNameSpanTest, FunctionCallArgumentSpans)
    {
        std::string code = "let res = compute(source: data, timeoutMs: 5000)";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);

        auto call = dynamic_cast<FunctionCall*>(assign->value.get());
        ASSERT_NE(call, nullptr);
        ASSERT_EQ(call->arguments.size(), 2);

        // Arg 0: "source: data"
        // "source" starts at col 19 (offset 18), len 6 -> col 25
        EXPECT_EQ(call->arguments[0].name, "source");
        assert_span(call->arguments[0].name.span, 1, 19, 1, 25, 18, 6);
        assert_span(call->arguments[0].span, 1, 19, 1, 31, 18, 12);

        // Arg 1: "timeoutMs: 5000"
        // "timeoutMs" starts at col 33 (offset 32), len 9 -> col 42
        EXPECT_EQ(call->arguments[1].name, "timeoutMs");
        assert_span(call->arguments[1].name.span, 1, 33, 1, 42, 32, 9);
        assert_span(call->arguments[1].span, 1, 33, 1, 48, 32, 15);
    }

    TEST_F(AstNameSpanTest, ModifierNameAndArgumentSpans)
    {
        std::string code = "@route(path: \"/users\", cache: true)\n"
                           "func getUsers() -> int { return 1 }";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->function_definitions.size(), 1);
        const auto& func = ast->function_definitions[0];
        ASSERT_EQ(func->modifiers.size(), 1);

        const auto& mod = func->modifiers[0];
        EXPECT_EQ(mod.name, "route");
        // "route": line 1, col 2 (offset 1), len 5 -> col 7
        assert_span(mod.name.span, 1, 2, 1, 7, 1, 5);
        // Full modifier span: "@route(path: \"/users\", cache: true)" -> line 1, col 1 -> 36, offset 0, len 35
        assert_span(mod.span, 1, 1, 1, 36, 0, 35);

        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: "path: \"/users\""
        // "path" starts at col 8 (offset 7), len 4 -> col 12
        EXPECT_EQ(mod.arguments[0].name, "path");
        assert_span(mod.arguments[0].name.span, 1, 8, 1, 12, 7, 4);
        assert_span(mod.arguments[0].span, 1, 8, 1, 22, 7, 14);

        // Arg 1: "cache: true"
        // "cache" starts at col 24 (offset 23), len 5 -> col 29
        EXPECT_EQ(mod.arguments[1].name, "cache");
        assert_span(mod.arguments[1].name.span, 1, 24, 1, 29, 23, 5);
        assert_span(mod.arguments[1].span, 1, 24, 1, 35, 23, 11);
    }

    TEST_F(AstNameSpanTest, DictItemDiverseKeysAndSpans)
    {
        std::string code = "let map = { userName: 1, age: 2, isActive: true }";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);

        auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
        ASSERT_NE(dict, nullptr);
        ASSERT_EQ(dict->elements.size(), 3);

        // Item 0: "userName: 1"
        // "userName" starts at col 13 (offset 12), len 8 -> col 21
        EXPECT_EQ(dict->elements[0].key, "userName");
        assert_span(dict->elements[0].key.span, 1, 13, 1, 21, 12, 8);
        assert_span(dict->elements[0].span, 1, 13, 1, 24, 12, 11);

        // Item 1: "age: 2"
        // "age" starts at col 26 (offset 25), len 3 -> col 29
        EXPECT_EQ(dict->elements[1].key, "age");
        assert_span(dict->elements[1].key.span, 1, 26, 1, 29, 25, 3);
        assert_span(dict->elements[1].span, 1, 26, 1, 32, 25, 6);

        // Item 2: "isActive: true"
        // "isActive" starts at col 34 (offset 33), len 8 -> col 42
        EXPECT_EQ(dict->elements[2].key, "isActive");
        assert_span(dict->elements[2].key.span, 1, 34, 1, 42, 33, 8);
        assert_span(dict->elements[2].span, 1, 34, 1, 48, 33, 14);
    }

    TEST_F(AstNameSpanTest, ImportStatementWithModifiersAndPathSpan)
    {
        std::string code = "@deprecated(since: \"2.0\")\n"
                           "import \"core/math\"";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->import_statements.size(), 1);
        const auto& imp = ast->import_statements[0];

        // "import \"core/math\"" on line 2
        // "\"core/math\"" starts at col 8 (offset 33), len 11 -> col 19
        EXPECT_EQ(imp->path, "\"core/math\"");
        assert_span(imp->path.span, 2, 8, 2, 19, 33, 11);

        ASSERT_EQ(imp->modifiers.size(), 1);
        const auto& mod = imp->modifiers[0];
        EXPECT_EQ(mod.name, "deprecated");
        // "deprecated": line 1, col 2 (offset 1), len 10 -> col 12
        assert_span(mod.name.span, 1, 2, 1, 12, 1, 10);

        ASSERT_EQ(mod.arguments.size(), 1);
        // "since: \"2.0\""
        // "since": line 1, col 13 (offset 12), len 5 -> col 18
        EXPECT_EQ(mod.arguments[0].name, "since");
        assert_span(mod.arguments[0].name.span, 1, 13, 1, 18, 12, 5);
        assert_span(mod.arguments[0].span, 1, 13, 1, 25, 12, 12);
    }

    TEST_F(AstNameSpanTest, TypeAnnotationNameAndGenericArgsSpans)
    {
        std::string code = "let map: Map<string, int> = 10";
        auto ast = parse_code(code);
        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        ASSERT_NE(assign, nullptr);
        ASSERT_EQ(assign->targets.size(), 1);

        const auto& type = assign->targets[0].type;
        ASSERT_NE(type, nullptr);

        // Outer type: "Map<string, int>" -> starts col 10 (offset 9) -> col 26 (len 16)
        EXPECT_EQ(type->name, "Map");
        assert_span(type->name.span, 1, 10, 1, 13, 9, 3);
        assert_span(type->span, 1, 10, 1, 26, 9, 16);

        ASSERT_EQ(type->generic_args.size(), 2);

        // Generic Arg 0: "string" -> starts col 14 (offset 13), len 6 -> col 20
        EXPECT_EQ(type->generic_args[0]->name, "string");
        assert_span(type->generic_args[0]->name.span, 1, 14, 1, 20, 13, 6);
        assert_span(type->generic_args[0]->span, 1, 14, 1, 20, 13, 6);

        // Generic Arg 1: "int" -> starts col 22 (offset 21), len 3 -> col 25
        EXPECT_EQ(type->generic_args[1]->name, "int");
        assert_span(type->generic_args[1]->name.span, 1, 22, 1, 25, 21, 3);
        assert_span(type->generic_args[1]->span, 1, 22, 1, 25, 21, 3);
    }
}
