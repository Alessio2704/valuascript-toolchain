#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <tuple>
#include <type_traits>

#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/categories/ast_category_types.h"
#include "ast_factory_test_reflection.h"

namespace valuascript::compiler::test
{
    struct AstSamplingTestDescriptor
    {
        std::string name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstSamplingTestDescriptor& desc)
        {
            return os << desc.name;
        }
    };

    template <typename T>
    inline void test_concrete_node_sampling_invariants()
    {
        constexpr int test_depth = 2;
        AstFactoryConfig cfg{};

        auto single = sample_node<T>(test_depth, cfg);
        const auto& single_ref = unwrap_node(single);
        EXPECT_TRUE(single_ref.is_valid());
        EXPECT_EQ(single_ref.span.column_start, (test_depth * 4) + 1);

        for (size_t count : {size_t{0}, size_t{1}, size_t{4}})
        {
            auto batch = sample_nodes<T>(count, test_depth, cfg);
            EXPECT_EQ(batch.size(), count);
            for (const auto& item : batch)
            {
                const auto& item_ref = unwrap_node(item);
                EXPECT_TRUE(item_ref.is_valid());
            }
        }

        AstFactoryConfig custom_cfg{};
        apply_string_to_node_config<T>(custom_cfg, "custom_pfx");
        auto custom_single = sample_node<T>(0, custom_cfg);
        const auto& custom_ref = unwrap_node(custom_single);
        EXPECT_TRUE(custom_ref.is_valid());
    }

    template <typename CategoryT>
    inline void test_category_node_sampling_invariants()
    {
        constexpr int test_depth = 1;
        AstFactoryConfig cfg{};

        auto single = sample_node<CategoryT>(test_depth, cfg);
        ASSERT_NE(single, nullptr);
        EXPECT_TRUE(single->is_valid());
        EXPECT_EQ(single->span.column_start, (test_depth * 4) + 1);

        for (size_t count : {size_t{0}, size_t{1}, size_t{4}})
        {
            auto batch = sample_nodes<CategoryT>(count, test_depth, cfg);
            EXPECT_EQ(batch.size(), count);
            for (const auto& item : batch)
            {
                ASSERT_NE(item, nullptr);
                EXPECT_TRUE(item->is_valid());
            }
        }
    }

    template <typename T>
    AstSamplingTestDescriptor make_concrete_sampling_test_descriptor()
    {
        return AstSamplingTestDescriptor{
            .name = std::string(get_ast_node_name<T>()),
            .run_test = []()
            {
                test_concrete_node_sampling_invariants<T>();
            }
        };
    }

    template <typename CategoryT>
    AstSamplingTestDescriptor make_category_sampling_test_descriptor()
    {
        std::string category_name;
        if constexpr (std::same_as<CategoryT, Statement>) { category_name = "Statement"; }
        else if constexpr (std::same_as<CategoryT, Expression>) { category_name = "Expression"; }
        else if constexpr (std::same_as<CategoryT, TypeAnnotation>) { category_name = "TypeAnnotation"; }

        return AstSamplingTestDescriptor{
            .name = std::move(category_name),
            .run_test = []()
            {
                test_category_node_sampling_invariants<CategoryT>();
            }
        };
    }

    inline std::vector<AstSamplingTestDescriptor> get_all_concrete_sampling_test_descriptors()
    {
        return collect_test_descriptors<AstSamplingTestDescriptor, AllAstNodeTypes>([]<typename T>()
        {
            return make_concrete_sampling_test_descriptor<T>();
        });
    }

    inline std::vector<AstSamplingTestDescriptor> get_all_category_sampling_test_descriptors()
    {
        return collect_test_descriptors<AstSamplingTestDescriptor, AstCategoryTypes>([]<typename CategoryT>()
        {
            return make_category_sampling_test_descriptor<CategoryT>();
        });
    }

    class AstConcreteNodeSamplingParameterizedTest : public testing::TestWithParam<AstSamplingTestDescriptor>
    {
    };

    TEST_P(AstConcreteNodeSamplingParameterizedTest, SampleNodeAndSampleNodesProduceValidNodes)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing sample_node<T> and sample_nodes<T> for Concrete Node: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    class AstCategorySamplingParameterizedTest : public testing::TestWithParam<AstSamplingTestDescriptor>
    {
    };

    TEST_P(AstCategorySamplingParameterizedTest, SampleCategoryNodeAndNodesProduceValidNodes)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing sample_node<Category> and sample_nodes<Category> for Category: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstSamplingTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstSamplingTestDescriptor>& info) const
        {
            return info.param.name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstConcreteNodeSamplingParameterizedTest,
        testing::ValuesIn(get_all_concrete_sampling_test_descriptors()),
        AstSamplingTestNameGenerator{}
    );

    INSTANTIATE_TEST_SUITE_P(
        AllCategories,
        AstCategorySamplingParameterizedTest,
        testing::ValuesIn(get_all_category_sampling_test_descriptors()),
        AstSamplingTestNameGenerator{}
    );
}
