#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "ast_rewriter_test_helper.h"
#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/equality/ast_equality.h"

namespace valuascript::compiler::test
{
    class UniversalReplacingRewriter : public AstRewriter
    {
    private:
        AstFactoryConfig config_;

        template <typename T>
        std::unique_ptr<T> create_replacement_ptr()
        {
            auto result = create_sample<T>(0, config_);
            auto_rewrite_children(*result);
            return result;
        }

        template <typename T>
        T create_replacement_val()
        {
            auto result = create_sample<T>(0, config_);
            auto_rewrite_children(result);
            return result;
        }

    public:
        using AstRewriter::rewrite;

        explicit UniversalReplacingRewriter(AstFactoryConfig cfg = {})
            : config_(std::move(cfg))
        {
        }

        StmtPtr rewrite(StmtPtr stmt) override
        {
            return stmt ? create_replacement_ptr<ExpressionStatement>() : nullptr;
        }

        ExprPtr rewrite(ExprPtr expr) override
        {
            return expr ? create_sample<StringLiteral>(0, config_) : nullptr;
        }

        TypeAnnPtr rewrite(TypeAnnPtr type) override
        {
            return type ? create_sample<TypeAnnotation>(0, config_) : nullptr;
        }

        ImportPtr rewrite(ImportPtr node) override
        {
            return node ? create_replacement_ptr<ImportStatement>() : nullptr;
        }

        DirectivePtr rewrite(DirectivePtr node) override
        {
            return node ? create_replacement_ptr<Directive>() : nullptr;
        }

        FuncDefPtr rewrite(FuncDefPtr node) override
        {
            return node ? create_replacement_ptr<FunctionDefinition>() : nullptr;
        }

        StructDefPtr rewrite(StructDefPtr node) override
        {
            return node ? create_replacement_ptr<StructDefinition>() : nullptr;
        }

        EnumDefPtr rewrite(EnumDefPtr node) override
        {
            return node ? create_replacement_ptr<EnumDefinition>() : nullptr;
        }

        TypeAliasPtr rewrite(TypeAliasPtr node) override
        {
            return node ? create_replacement_ptr<TypeAliasDefinition>() : nullptr;
        }

        ExtensionDefPtr rewrite(ExtensionDefPtr node) override
        {
            return node ? create_replacement_ptr<ExtensionDefinition>() : nullptr;
        }

        Modifier rewrite(Modifier) override { return create_replacement_val<Modifier>(); }
        FunctionParameter rewrite(FunctionParameter) override { return create_replacement_val<FunctionParameter>(); }
        StructField rewrite(StructField) override { return create_replacement_val<StructField>(); }
        EnumCase rewrite(EnumCase) override { return create_replacement_val<EnumCase>(); }
        SwitchCase rewrite(SwitchCase) override { return create_replacement_val<SwitchCase>(); }
        AssignmentTarget rewrite(AssignmentTarget) override { return create_replacement_val<AssignmentTarget>(); }
        CallArgument rewrite(CallArgument) override { return create_replacement_val<CallArgument>(); }
        DictItem rewrite(DictItem) override { return create_replacement_val<DictItem>(); }
        Comment rewrite(Comment) override { return create_replacement_val<Comment>(); }
    };

    template <typename NodeT, size_t Is>
    inline void verify_member_replaced(const NodeT& original, const NodeT& rewritten)
    {
        using MemberPtrT = std::tuple_element_t<Is, decltype(AstNodeSchema<NodeT>::members)>;
        using MemberRawT = std::remove_cvref_t<decltype(std::declval<NodeT>().*std::declval<MemberPtrT>())>;

        [[maybe_unused]] const auto& orig_member = original.*std::get<Is>(AstNodeSchema<NodeT>::members);
        [[maybe_unused]] const auto& rew_member = rewritten.*std::get<Is>(AstNodeSchema<NodeT>::members);

        if constexpr (IsUniquePtrOfAstNode<MemberRawT>)
        {
            if (orig_member)
            {
                ASSERT_NE(rew_member, nullptr);
                EXPECT_NE(rew_member.get(), orig_member.get());
                EXPECT_TRUE(rew_member->is_valid());
            }
        }
        else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberRawT>)
        {
            ASSERT_EQ(rew_member.size(), orig_member.size());
            for (size_t i = 0; i < orig_member.size(); ++i)
            {
                ASSERT_NE(rew_member[i], nullptr);
                EXPECT_NE(rew_member[i].get(), orig_member[i].get());
                EXPECT_TRUE(rew_member[i]->is_valid());
            }
        }
        else if constexpr (IsInnerAstNode<MemberRawT>)
        {
            EXPECT_TRUE(rew_member.is_valid());
            EXPECT_FALSE(ast_equals(rew_member, orig_member));
        }
        else if constexpr (IsVectorOfInnerAstNode<MemberRawT>)
        {
            ASSERT_EQ(rew_member.size(), orig_member.size());
            for (size_t i = 0; i < orig_member.size(); ++i)
            {
                EXPECT_TRUE(rew_member[i].is_valid());
                EXPECT_FALSE(ast_equals(rew_member[i], orig_member[i]));
            }
        }
        else if constexpr (IsOptionalAstFieldOfAstNode<MemberRawT>)
        {
            if (orig_member.has_value())
            {
                ASSERT_TRUE(rew_member.has_value());
                using ValT = typename MemberRawT::value_type;
                if constexpr (is_unique_ptr_v<ValT>)
                {
                    ASSERT_NE(rew_member.get(), nullptr);
                    EXPECT_NE(rew_member.get(), orig_member.get());
                    EXPECT_TRUE(rew_member->is_valid());
                }
                else
                {
                    EXPECT_TRUE(rew_member.value().is_valid());
                    EXPECT_FALSE(ast_equals(rew_member.value(), orig_member.value()));
                }
            }
        }
    }

    template <typename NodeT>
    inline void run_schema_driven_replacement_test()
    {
        AstFactoryConfig config;
        config.function_parameter.default_value.value = true;
        config.enum_case.value.value = true;
        config.type_annotation.generic_args.count = 2;
        config.tuple_type_annotation.generic_args.count = 2;
        config.tuple_type_annotation.element_types.count = 2;

        auto sample = create_sample<NodeT>(2, config);
        UniversalReplacingRewriter rewriter(config);

        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            auto sample_copy = clone_node(sample);
            auto rewritten = rewriter.rewrite(std::move(sample_copy));

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>)
            {
                (verify_member_replaced<NodeT, Is>(sample, rewritten), ...);
            }(std::make_index_sequence<N>{});
        }
        else
        {
            ASSERT_NE(sample, nullptr);
            auto sample_copy = clone_node(sample);
            rewriter.auto_rewrite_children(*sample);

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>)
            {
                (verify_member_replaced<NodeT, Is>(*sample_copy, *sample), ...);
            }(std::make_index_sequence<N>{});
        }
    }

    template <typename NodeT>
    struct AstRewriterPropertySweepTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_schema_driven_replacement_test<NodeT>();
                }
            };
        }
    };

    class AstRewriterPropertySweepParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    TEST_P(AstRewriterPropertySweepParameterizedTest, SweepsAndVerifiesAllChildMembersInSchema)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Schema Member Sweep for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstRewriterPropertySweepParameterizedTest,
        testing::ValuesIn(
            (AstRewriterDescriptorCollector<AllAstNodeTypes, AstRewriterPropertySweepTestBuilder>::collect())
        ),
        AstRewriterTestNameGenerator{}
    );
}
