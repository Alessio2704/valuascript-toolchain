#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <utility>

#include "ast_rewriter_test_helper.h"
#include "ast/factory/ast_factory.h"

namespace valuascript::compiler::test
{
    inline AstFactoryConfig create_inner_node_sentinel_config()
    {
        AstFactoryConfig cfg;
        cfg.string_literal.value.value = "__inner_expr_transformed__";
        cfg.type_annotation.name.prefix = "__inner_type_transformed__";
        cfg.modifier.name.prefix = "__transformed_mod__";
        cfg.comment.text.value = "__transformed_comment__";
        return cfg;
    }

    class DeepInnerNodePropertyRewriter : public AstRewriter
    {
    private:
        AstFactoryConfig config_ = create_inner_node_sentinel_config();

    public:
        using AstRewriter::rewrite;

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

        virtual Modifier rewrite(Modifier) override
        {
            auto result = create_sample<Modifier>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

        virtual Comment rewrite(Comment) override
        {
            return create_sample<Comment>(0, config_);
        }
    };

    template <typename InnerNodeT>
    inline void run_deep_inner_node_test()
    {
        AstRewriter default_rewriter;
        AstFactoryConfig config;
        config.function_parameter.default_value.value = true;
        config.enum_case.value.value = true;

        auto sample = create_sample<InnerNodeT>(2, config);
        auto cloned = clone_node(sample);

        auto identity_rewritten = default_rewriter.rewrite(std::move(cloned));
        EXPECT_TRUE(ast_equals(identity_rewritten, sample));

        DeepInnerNodePropertyRewriter transformer;
        auto sample_copy = clone_node(sample);
        auto transformed = transformer.rewrite(std::move(sample_copy));

        if constexpr (std::same_as<InnerNodeT, FunctionParameter>)
        {
            if (transformed.type)
            {
                EXPECT_TRUE(transformed.type->name.value.starts_with("__inner_type_transformed__"));
            }
            if (transformed.default_value)
            {
                auto* str = dynamic_cast<StringLiteral*>(transformed.default_value.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__inner_expr_transformed__");
            }
        }
        else if constexpr (std::same_as<InnerNodeT, StructField>)
        {
            if (transformed.type)
            {
                EXPECT_TRUE(transformed.type->name.value.starts_with("__inner_type_transformed__"));
            }
        }
        else if constexpr (std::same_as<InnerNodeT, EnumCase>)
        {
            if (transformed.value)
            {
                auto* str = dynamic_cast<StringLiteral*>(transformed.value.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__inner_expr_transformed__");
            }
        }
        else if constexpr (std::same_as<InnerNodeT, SwitchCase>)
        {
            if (transformed.result)
            {
                auto* str = dynamic_cast<StringLiteral*>(transformed.result.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__inner_expr_transformed__");
            }
        }
        else if constexpr (std::same_as<InnerNodeT, AssignmentTarget>)
        {
            if (transformed.type)
            {
                EXPECT_TRUE(transformed.type->name.value.starts_with("__inner_type_transformed__"));
            }
        }
        else if constexpr (std::same_as<InnerNodeT, CallArgument>)
        {
            if (transformed.value)
            {
                auto* str = dynamic_cast<StringLiteral*>(transformed.value.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__inner_expr_transformed__");
            }
        }
        else if constexpr (std::same_as<InnerNodeT, DictItem>)
        {
            if (transformed.value)
            {
                auto* str = dynamic_cast<StringLiteral*>(transformed.value.get());
                ASSERT_NE(str, nullptr);
                EXPECT_EQ(str->value, "__inner_expr_transformed__");
            }
        }
        else if constexpr (std::same_as<InnerNodeT, Modifier>)
        {
            EXPECT_TRUE(transformed.name.value.starts_with("__transformed_mod__"));
        }
        else if constexpr (std::same_as<InnerNodeT, Comment>)
        {
            EXPECT_EQ(transformed.text, "__transformed_comment__");
        }
    }

    template <typename InnerNodeT>
    struct InnerNodeDeepRewriteTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<InnerNodeT>()),
                .run_test = []()
                {
                    run_deep_inner_node_test<InnerNodeT>();
                }
            };
        }
    };

    class InnerNodeDeepRewriteParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    TEST_P(InnerNodeDeepRewriteParameterizedTest, DeeplyTransformsInnerNodeChildFields)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Deep Inner Node Rewrite for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllInnerNodes,
        InnerNodeDeepRewriteParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllInnerNodeTypes, InnerNodeDeepRewriteTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );
}
