#include <gtest/gtest.h>
#include "ast/ast.h"

namespace valuascript::compiler::test
{
    TEST(AstCloneTest, NullPointerReturnsNull)
    {
        Expression* null_expr = nullptr;
        Statement* null_stmt = nullptr;
        TypeAnnotation* null_type = nullptr;
        AstNode* null_node = nullptr;

        EXPECT_EQ(clone_node(null_expr), nullptr);
        EXPECT_EQ(clone_node(null_stmt), nullptr);
        EXPECT_EQ(clone_node(null_type), nullptr);
        EXPECT_EQ(clone_node(null_node), nullptr);

        std::unique_ptr<Expression> null_expr_ptr = nullptr;
        EXPECT_EQ(clone_node(null_expr_ptr), nullptr);
    }

    TEST(AstCloneTest, CloneLiteralsAndBasicExpressions)
    {
        SourceSpan span{.line_start = 1, .column_start = 2, .line_end = 1, .column_end = 5};

        auto num = std::make_unique<NumberLiteral>("42");
        num->span = span;
        auto num_clone = clone_node(num);
        ASSERT_NE(num_clone, nullptr);
        EXPECT_NE(num_clone.get(), num.get());
        EXPECT_EQ(num_clone->value, "42");
        EXPECT_EQ(num_clone->span.line_start, 1);
        EXPECT_EQ(num_clone->span.column_end, 5);

        auto str = std::make_unique<StringLiteral>("hello");
        str->span = span;
        auto str_clone = clone_node(str);
        ASSERT_NE(str_clone, nullptr);
        EXPECT_EQ(str_clone->value, "hello");
        EXPECT_EQ(str_clone->span.line_start, 1);

        auto b = std::make_unique<BooleanLiteral>(true);
        auto b_clone = clone_node(b);
        ASSERT_NE(b_clone, nullptr);
        EXPECT_EQ(b_clone->value, true);

        auto pct = std::make_unique<PercentageLiteral>("50%");
        auto pct_clone = clone_node(pct);
        ASSERT_NE(pct_clone, nullptr);
        EXPECT_EQ(pct_clone->value, "50%");

        auto id = std::make_unique<IdentifierAccess>(NodeName{"my_var", span});
        auto id_clone = clone_node(id);
        ASSERT_NE(id_clone, nullptr);
        EXPECT_EQ(id_clone->name.value, "my_var");
        EXPECT_EQ(id_clone->name.span.line_start, 1);

        auto self = std::make_unique<SelfExpression>();
        auto self_clone = clone_node(self);
        ASSERT_NE(self_clone, nullptr);
        EXPECT_EQ(self_clone->kind, AstKind::SelfExpression);
    }

    TEST(AstCloneTest, CloneNestedExpressionsAndIndependence)
    {
        auto left = std::make_unique<NumberLiteral>("10");
        auto right = std::make_unique<NumberLiteral>("20");
        auto bin = std::make_unique<BinaryExpression>(std::move(left), TokenType::Plus, std::move(right));
        bin->span = SourceSpan{.line_start = 1, .column_start = 1, .line_end = 1, .column_end = 7};

        auto bin_clone = clone_node(bin);
        ASSERT_NE(bin_clone, nullptr);
        EXPECT_NE(bin_clone.get(), bin.get());
        EXPECT_EQ(bin_clone->op, TokenType::Plus);
        ASSERT_NE(bin_clone->left, nullptr);
        ASSERT_NE(bin_clone->right, nullptr);
        EXPECT_NE(bin_clone->left.get(), bin->left.get());
        EXPECT_NE(bin_clone->right.get(), bin->right.get());
        EXPECT_EQ(ast_cast<NumberLiteral>(bin_clone->left.get())->value, "10");
        EXPECT_EQ(ast_cast<NumberLiteral>(bin_clone->right.get())->value, "20");

        auto cond = std::make_unique<BooleanLiteral>(true);
        auto then_b = std::make_unique<NumberLiteral>("1");
        auto else_b = std::make_unique<NumberLiteral>("2");
        auto ternary = std::make_unique<ConditionalExpression>(
            std::move(cond), std::move(then_b), std::move(else_b));

        auto ternary_clone = clone_node(ternary);
        ASSERT_NE(ternary_clone, nullptr);
        EXPECT_NE(ternary_clone.get(), ternary.get());
        EXPECT_NE(ternary_clone->condition.get(), ternary->condition.get());
        EXPECT_NE(ternary_clone->then_branch.get(), ternary->then_branch.get());
        EXPECT_NE(ternary_clone->else_branch.get(), ternary->else_branch.get());
    }

    TEST(AstCloneTest, CloneComplexExpressions)
    {
        std::vector<CallArgument> args;
        args.push_back(CallArgument(NodeName{"arg1"}, std::make_unique<NumberLiteral>("100")));
        auto call = std::make_unique<FunctionCall>(
            std::make_unique<IdentifierAccess>(NodeName{"foo"}),
            std::move(args)
        );

        auto call_clone = clone_node(call);
        ASSERT_NE(call_clone, nullptr);
        EXPECT_NE(call_clone.get(), call.get());
        EXPECT_EQ(ast_cast<IdentifierAccess>(call_clone->target.get())->name.value, "foo");
        ASSERT_EQ(call_clone->arguments.size(), 1);
        EXPECT_EQ(call_clone->arguments[0].name.value, "arg1");
        EXPECT_NE(call_clone->arguments[0].value.get(), call->arguments[0].value.get());
        EXPECT_EQ(ast_cast<NumberLiteral>(call_clone->arguments[0].value.get())->value, "100");

        std::vector<DictItem> items;
        items.push_back(DictItem({}, NodeName{"key1"}, std::make_unique<StringLiteral>("val1")));
        auto dict = std::make_unique<DictLiteral>(std::move(items));

        auto dict_clone = clone_node(dict);
        ASSERT_NE(dict_clone, nullptr);
        ASSERT_EQ(dict_clone->elements.size(), 1);
        EXPECT_EQ(dict_clone->elements[0].key.value, "key1");
        EXPECT_EQ(ast_cast<StringLiteral>(dict_clone->elements[0].value.get())->value, "val1");

        std::vector<SwitchCase> cases;
        cases.push_back(SwitchCase({}, {NodeName{"A"}}, std::make_unique<NumberLiteral>("1")));
        auto sw = std::make_unique<SwitchExpression>(
            std::make_unique<IdentifierAccess>(NodeName{"x"}),
            std::move(cases),
            std::vector<Modifier>{},
            std::make_unique<NumberLiteral>("0")
        );

        auto sw_clone = clone_node(sw);
        ASSERT_NE(sw_clone, nullptr);
        ASSERT_EQ(sw_clone->cases.size(), 1);
        EXPECT_EQ(sw_clone->cases[0].identifiers[0].value, "A");
        EXPECT_EQ(ast_cast<NumberLiteral>(sw_clone->default_case.get())->value, "0");
    }

    TEST(AstCloneTest, CloneStatementsAndDeclarations)
    {
        std::vector<Modifier> mods;
        mods.push_back(Modifier(NodeName{"pure"}));

        std::vector<FunctionParameter> params;
        params.push_back(FunctionParameter(
            {},
            NodeName{"a"},
            std::make_unique<TypeAnnotation>(NodeName{"int"}),
            std::make_unique<NumberLiteral>("0")
        ));

        std::vector<TypeAnnPtr> ret_types;
        ret_types.push_back(std::make_unique<TypeAnnotation>(NodeName{"int"}));

        std::vector<StmtPtr> body;
        std::vector<ExprPtr> ret_vals;
        ret_vals.push_back(std::make_unique<IdentifierAccess>(NodeName{"a"}));
        body.push_back(std::make_unique<ReturnStatement>(std::vector<Modifier>{}, std::move(ret_vals)));

        auto func = std::make_unique<FunctionDefinition>(
            std::move(mods),
            NodeName{"compute"},
            std::move(params),
            std::move(ret_types),
            std::move(body),
            std::optional<std::string>{"Documentation"}
        );

        auto func_clone = clone_node(func);
        ASSERT_NE(func_clone, nullptr);
        EXPECT_NE(func_clone.get(), func.get());
        EXPECT_EQ(func_clone->name.value, "compute");
        EXPECT_EQ(func_clone->docstring, "Documentation");
        ASSERT_EQ(func_clone->modifiers.size(), 1);
        EXPECT_EQ(func_clone->modifiers[0].name.value, "pure");
        ASSERT_EQ(func_clone->parameters.size(), 1);
        EXPECT_EQ(func_clone->parameters[0].name.value, "a");
        EXPECT_NE(func_clone->parameters[0].type.get(), func->parameters[0].type.get());
        EXPECT_NE(func_clone->parameters[0].default_value.get(), func->parameters[0].default_value.get());
        ASSERT_EQ(func_clone->body.size(), 1);
        EXPECT_NE(func_clone->body[0].get(), func->body[0].get());
    }

    TEST(AstCloneTest, CloneModifiersAndElements)
    {
        Modifier mod(NodeName{"timeout"});
        mod.arguments.push_back(CallArgument(NodeName{"seconds"}, std::make_unique<NumberLiteral>("5")));

        Modifier mod_clone = clone_node(mod);
        EXPECT_EQ(mod_clone.name.value, "timeout");
        ASSERT_EQ(mod_clone.arguments.size(), 1);
        EXPECT_EQ(mod_clone.arguments[0].name.value, "seconds");
        EXPECT_NE(mod_clone.arguments[0].value.get(), mod.arguments[0].value.get());
        EXPECT_EQ(ast_cast<NumberLiteral>(mod_clone.arguments[0].value.get())->value, "5");

        std::vector<Modifier> target_mods;
        target_mods.push_back(Modifier(NodeName{"exported"}));
        AssignmentTarget target(
            std::move(target_mods),
            NodeName{"x"},
            std::make_unique<TypeAnnotation>(NodeName{"float"})
        );
        AssignmentTarget target_clone = clone_node(target);
        EXPECT_EQ(target_clone.name.value, "x");
        ASSERT_EQ(target_clone.modifiers.size(), 1);
        EXPECT_EQ(target_clone.modifiers[0].name.value, "exported");
        ASSERT_NE(target_clone.type, nullptr);
        EXPECT_EQ(target_clone.type->name.value, "float");
        EXPECT_NE(target_clone.type.get(), target.type.get());
    }

    TEST(AstCloneTest, CloneProgram)
    {
        auto program = std::make_unique<Program>();
        program->comments.push_back(Comment("/* file header */"));
        program->directives.push_back(std::make_unique<Directive>(
            NodeName{"optimize"},
            std::make_unique<BooleanLiteral>(true)
        ));
        program->type_aliases.push_back(std::make_unique<TypeAliasDefinition>(
            std::vector<Modifier>{},
            NodeName{"ID"},
            std::make_unique<TypeAnnotation>(NodeName{"int"})
        ));

        auto program_clone = clone_node(program);
        ASSERT_NE(program_clone, nullptr);
        EXPECT_NE(program_clone.get(), program.get());
        ASSERT_EQ(program_clone->comments.size(), 1);
        EXPECT_EQ(program_clone->comments[0].text, "/* file header */");
        ASSERT_EQ(program_clone->directives.size(), 1);
        EXPECT_EQ(program_clone->directives[0]->name.value, "optimize");
        ASSERT_EQ(program_clone->type_aliases.size(), 1);
        EXPECT_EQ(program_clone->type_aliases[0]->name.value, "ID");
    }
}
