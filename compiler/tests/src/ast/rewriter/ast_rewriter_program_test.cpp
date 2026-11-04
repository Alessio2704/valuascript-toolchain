#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>

#include "ast_rewriter_test_helper.h"
#include "ast/walker/ast_walker.h"
#include "ast/factory/ast_factory.h"

namespace valuascript::compiler::test
{
    inline AstFactoryConfig create_exhaustive_program_config()
    {
        AstFactoryConfig cfg;
        cfg.function_parameter.default_value.value = true;
        cfg.enum_case.value.value = true;
        cfg.type_annotation.generic_args.count = 2;
        cfg.tuple_type_annotation.generic_args.count = 2;
        cfg.tuple_type_annotation.element_types.count = 2;

        cfg.program.comments.count = 2;
        cfg.program.import_statements.count = 2;
        cfg.program.directives.count = 2;
        cfg.program.execution_steps.count = 2;
        cfg.program.function_definitions.count = 2;
        cfg.program.struct_definitions.count = 2;
        cfg.program.enum_definitions.count = 2;
        cfg.program.type_aliases.count = 2;
        cfg.program.extension_definitions.count = 2;

        cfg.extension_definition.execution_steps.count = 2;
        cfg.extension_definition.function_definitions.count = 2;
        cfg.extension_definition.struct_definitions.count = 2;
        cfg.extension_definition.enum_definitions.count = 2;
        cfg.extension_definition.type_aliases.count = 2;

        cfg.function_definition.parameters.count = 2;
        cfg.function_definition.return_types.count = 2;
        cfg.function_definition.body.count = 2;

        cfg.struct_definition.fields.count = 2;
        cfg.enum_definition.cases.count = 2;

        return cfg;
    }

    inline AstFactoryConfig create_program_sentinel_config()
    {
        AstFactoryConfig cfg;
        cfg.general.expression_kind = ExpressionKind::from<StringLiteral>();
        cfg.general.statement_kind = StatementKind::from<ExpressionStatement>();
        cfg.string_literal.value.value = "__sentinel_expr__";
        cfg.type_annotation.name.prefix = "__sentinel_type__";
        cfg.function_definition.name.prefix = "__sentinel_func__";
        cfg.struct_definition.name.prefix = "__sentinel_struct__";
        cfg.enum_definition.name.prefix = "__sentinel_enum__";
        cfg.type_alias_definition.name.prefix = "__sentinel_alias__";
        cfg.import_statement.path.prefix = "__sentinel_import__";
        cfg.directive.name.prefix = "__sentinel_dir__";
        cfg.modifier.name.prefix = "__sentinel_mod__";
        cfg.function_parameter.name.prefix = "__sentinel_param__";
        cfg.struct_field.name.prefix = "__sentinel_field__";
        cfg.enum_case.name.prefix = "__sentinel_case__";
        cfg.assignment_target.name.prefix = "__sentinel_target__";
        cfg.call_argument.name.prefix = "__sentinel_arg__";
        cfg.dict_item.key.prefix = "__sentinel_key__";
        cfg.comment.text.value = "__sentinel_comment__";
        return cfg;
    }

    class FullProgramUniversalRewriter : public AstRewriter
    {
    private:
        AstFactoryConfig config_ = create_program_sentinel_config();

    public:
        using AstRewriter::rewrite;

        StmtPtr rewrite(StmtPtr stmt) override
        {
            if (!stmt) return nullptr;
            auto result = create_sample<ExpressionStatement>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        ExprPtr rewrite(ExprPtr expr) override
        {
            if (!expr) return nullptr;
            return create_sample<StringLiteral>(0, config_);
        }

        TypeAnnPtr rewrite(TypeAnnPtr type) override
        {
            if (!type) return nullptr;
            return create_sample<TypeAnnotation>(0, config_);
        }

        ImportPtr rewrite(ImportPtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<ImportStatement>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        DirectivePtr rewrite(DirectivePtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<Directive>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        FuncDefPtr rewrite(FuncDefPtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<FunctionDefinition>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        StructDefPtr rewrite(StructDefPtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<StructDefinition>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        EnumDefPtr rewrite(EnumDefPtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<EnumDefinition>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        TypeAliasPtr rewrite(TypeAliasPtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<TypeAliasDefinition>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        ExtensionDefPtr rewrite(ExtensionDefPtr node) override
        {
            if (!node) return nullptr;
            auto result = create_sample<ExtensionDefinition>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        virtual Modifier rewrite(Modifier) override
        {
            auto result = create_sample<Modifier>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual FunctionParameter rewrite(FunctionParameter) override
        {
            auto result = create_sample<FunctionParameter>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual StructField rewrite(StructField) override
        {
            auto result = create_sample<StructField>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual EnumCase rewrite(EnumCase) override
        {
            auto result = create_sample<EnumCase>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual SwitchCase rewrite(SwitchCase) override
        {
            auto result = create_sample<SwitchCase>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual AssignmentTarget rewrite(AssignmentTarget) override
        {
            auto result = create_sample<AssignmentTarget>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual CallArgument rewrite(CallArgument) override
        {
            auto result = create_sample<CallArgument>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual DictItem rewrite(DictItem) override
        {
            auto result = create_sample<DictItem>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual Comment rewrite(Comment) override
        {
            auto result = create_sample<Comment>(0, config_);
            auto_rewrite_children(result);
            return result;
        }
    };

    class ExpressionOnlyProgramRewriter : public AstRewriter
    {
    private:
        AstFactoryConfig config_ = create_program_sentinel_config();

    public:
        using AstRewriter::rewrite;

        ExprPtr rewrite(ExprPtr expr) override
        {
            if (!expr) return nullptr;
            return create_sample<StringLiteral>(0, config_);
        }
    };

    class TypeAnnotationOnlyProgramRewriter : public AstRewriter
    {
    private:
        AstFactoryConfig config_ = create_program_sentinel_config();

    public:
        using AstRewriter::rewrite;

        TypeAnnPtr rewrite(TypeAnnPtr type) override
        {
            if (!type) return nullptr;
            return create_sample<TypeAnnotation>(0, config_);
        }
    };

    TEST(AstRewriterProgramTest, IdentityRewritePreservesFullProgramHierarchy)
    {
        auto program_config = create_exhaustive_program_config();
        auto program = create_sample_program(2, program_config);
        ASSERT_NE(program, nullptr);
        auto cloned = clone_node(program);

        AstRewriter rewriter;
        auto rewritten = rewriter.rewrite(std::move(cloned));
        ASSERT_NE(rewritten, nullptr);
        EXPECT_TRUE(ast_equals(rewritten, program));
    }

    TEST(AstRewriterProgramTest, DeepTransformationRewritesAllProgramContainersAllTogether)
    {
        auto program_config = create_exhaustive_program_config();
        auto program = create_sample_program(2, program_config);
        ASSERT_NE(program, nullptr);

        ASSERT_FALSE(program->comments.empty());
        ASSERT_FALSE(program->import_statements.empty());
        ASSERT_FALSE(program->directives.empty());
        ASSERT_FALSE(program->execution_steps.empty());
        ASSERT_FALSE(program->function_definitions.empty());
        ASSERT_FALSE(program->struct_definitions.empty());
        ASSERT_FALSE(program->enum_definitions.empty());
        ASSERT_FALSE(program->type_aliases.empty());
        ASSERT_FALSE(program->extension_definitions.empty());

        FullProgramUniversalRewriter transformer;
        auto rewritten = transformer.rewrite(std::move(program));
        ASSERT_NE(rewritten, nullptr);

        for (const auto& comm : rewritten->comments)
        {
            EXPECT_EQ(comm.text, "__sentinel_comment__");
        }

        for (const auto& imp : rewritten->import_statements)
        {
            EXPECT_TRUE(imp->path.value.starts_with("__sentinel_import__"));
            for (const auto& mod : imp->modifiers)
            {
                EXPECT_TRUE(mod.name.value.starts_with("__sentinel_mod__"));
            }
        }

        for (const auto& dir : rewritten->directives)
        {
            EXPECT_TRUE(dir->name.value.starts_with("__sentinel_dir__"));
            ASSERT_NE(dir->value, nullptr);
            auto* str = dynamic_cast<const StringLiteral*>(dir->value.get());
            ASSERT_NE(str, nullptr);
            EXPECT_EQ(str->value, "__sentinel_expr__");
        }

        for (const auto& stmt : rewritten->execution_steps)
        {
            ASSERT_NE(stmt, nullptr);
            auto* expr_stmt = dynamic_cast<const ExpressionStatement*>(stmt.get());
            ASSERT_NE(expr_stmt, nullptr);
            auto* str = dynamic_cast<const StringLiteral*>(expr_stmt->expr.get());
            ASSERT_NE(str, nullptr);
            EXPECT_EQ(str->value, "__sentinel_expr__");
        }

        for (const auto& fn : rewritten->function_definitions)
        {
            EXPECT_TRUE(fn->name.value.starts_with("__sentinel_func__"));
            for (const auto& mod : fn->modifiers)
            {
                EXPECT_TRUE(mod.name.value.starts_with("__sentinel_mod__"));
            }
            for (const auto& param : fn->parameters)
            {
                EXPECT_TRUE(param.name.value.starts_with("__sentinel_param__"));
                if (param.type)
                {
                    EXPECT_TRUE(param.type->name.value.starts_with("__sentinel_type__"));
                }
                if (param.default_value)
                {
                    auto* str = dynamic_cast<const StringLiteral*>(param.default_value.get());
                    ASSERT_NE(str, nullptr);
                    EXPECT_EQ(str->value, "__sentinel_expr__");
                }
            }
            for (const auto& ret : fn->return_types)
            {
                EXPECT_TRUE(ret->name.value.starts_with("__sentinel_type__"));
            }
            for (const auto& body_stmt : fn->body)
            {
                auto* expr_stmt = dynamic_cast<const ExpressionStatement*>(body_stmt.get());
                ASSERT_NE(expr_stmt, nullptr);
                auto* str = dynamic_cast<const StringLiteral*>(expr_stmt->expr.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__sentinel_expr__");
            }
        }

        for (const auto& st : rewritten->struct_definitions)
        {
            EXPECT_TRUE(st->name.value.starts_with("__sentinel_struct__"));
            for (const auto& mod : st->modifiers)
            {
                EXPECT_TRUE(mod.name.value.starts_with("__sentinel_mod__"));
            }
            for (const auto& field : st->fields)
            {
                EXPECT_TRUE(field.name.value.starts_with("__sentinel_field__"));
                if (field.type)
                {
                    EXPECT_TRUE(field.type->name.value.starts_with("__sentinel_type__"));
                }
            }
        }

        for (const auto& en : rewritten->enum_definitions)
        {
            EXPECT_TRUE(en->name.value.starts_with("__sentinel_enum__"));
            for (const auto& mod : en->modifiers)
            {
                EXPECT_TRUE(mod.name.value.starts_with("__sentinel_mod__"));
            }
            if (en->underlying_type)
            {
                EXPECT_TRUE(en->underlying_type->name.value.starts_with("__sentinel_type__"));
            }
            for (const auto& enum_case : en->cases)
            {
                EXPECT_TRUE(enum_case.name.value.starts_with("__sentinel_case__"));
                if (enum_case.value)
                {
                    auto* str = dynamic_cast<const StringLiteral*>(enum_case.value.get());
                    ASSERT_NE(str, nullptr);
                    EXPECT_EQ(str->value, "__sentinel_expr__");
                }
            }
        }

        for (const auto& ta : rewritten->type_aliases)
        {
            EXPECT_TRUE(ta->name.value.starts_with("__sentinel_alias__"));
            for (const auto& mod : ta->modifiers)
            {
                EXPECT_TRUE(mod.name.value.starts_with("__sentinel_mod__"));
            }
            if (ta->target_type)
            {
                EXPECT_TRUE(ta->target_type->name.value.starts_with("__sentinel_type__"));
            }
        }

        for (const auto& ext : rewritten->extension_definitions)
        {
            for (const auto& mod : ext->modifiers)
            {
                EXPECT_TRUE(mod.name.value.starts_with("__sentinel_mod__"));
            }
            if (ext->target_type)
            {
                EXPECT_TRUE(ext->target_type->name.value.starts_with("__sentinel_type__"));
            }
            for (const auto& ext_stmt : ext->execution_steps)
            {
                auto* expr_stmt = dynamic_cast<const ExpressionStatement*>(ext_stmt.get());
                ASSERT_NE(expr_stmt, nullptr);
                auto* str = dynamic_cast<const StringLiteral*>(expr_stmt->expr.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__sentinel_expr__");
            }
            for (const auto& ext_fn : ext->function_definitions)
            {
                EXPECT_TRUE(ext_fn->name.value.starts_with("__sentinel_func__"));
            }
            for (const auto& ext_st : ext->struct_definitions)
            {
                EXPECT_TRUE(ext_st->name.value.starts_with("__sentinel_struct__"));
            }
            for (const auto& ext_en : ext->enum_definitions)
            {
                EXPECT_TRUE(ext_en->name.value.starts_with("__sentinel_enum__"));
            }
            for (const auto& ext_ta : ext->type_aliases)
            {
                EXPECT_TRUE(ext_ta->name.value.starts_with("__sentinel_alias__"));
            }
        }
    }

    TEST(AstRewriterProgramTest, TargetedExpressionRewriterLeavesOuterProgramStructureIntact)
    {
        auto program_config = create_exhaustive_program_config();
        auto program = create_sample_program(2, program_config);
        ASSERT_NE(program, nullptr);

        auto original_func_name = program->function_definitions.front()->name.value;
        auto original_struct_name = program->struct_definitions.front()->name.value;

        ExpressionOnlyProgramRewriter rewriter;
        auto rewritten = rewriter.rewrite(std::move(program));
        ASSERT_NE(rewritten, nullptr);

        EXPECT_EQ(rewritten->function_definitions.front()->name.value, original_func_name);
        EXPECT_EQ(rewritten->struct_definitions.front()->name.value, original_struct_name);

        for (const auto& dir : rewritten->directives)
        {
            auto* str = dynamic_cast<const StringLiteral*>(dir->value.get());
            ASSERT_NE(str, nullptr);
            EXPECT_EQ(str->value, "__sentinel_expr__");
        }
    }

    TEST(AstRewriterProgramTest, TargetedTypeRewriterTransformsAllTypesInProgramHierarchy)
    {
        auto program_config = create_exhaustive_program_config();
        auto program = create_sample_program(2, program_config);
        ASSERT_NE(program, nullptr);

        TypeAnnotationOnlyProgramRewriter rewriter;
        auto rewritten = rewriter.rewrite(std::move(program));
        ASSERT_NE(rewritten, nullptr);

        for (const auto& fn : rewritten->function_definitions)
        {
            for (const auto& param : fn->parameters)
            {
                if (param.type)
                {
                    EXPECT_TRUE(param.type->name.value.starts_with("__sentinel_type__"));
                }
            }
            for (const auto& ret : fn->return_types)
            {
                EXPECT_TRUE(ret->name.value.starts_with("__sentinel_type__"));
            }
        }

        for (const auto& st : rewritten->struct_definitions)
        {
            for (const auto& field : st->fields)
            {
                if (field.type)
                {
                    EXPECT_TRUE(field.type->name.value.starts_with("__sentinel_type__"));
                }
            }
        }

        for (const auto& ta : rewritten->type_aliases)
        {
            if (ta->target_type)
            {
                EXPECT_TRUE(ta->target_type->name.value.starts_with("__sentinel_type__"));
            }
        }
    }
}
