#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <array>
#include <type_traits>
#include <utility>

#include "ast/factory/ast_factory.h"
#include "ast/categories/ast_category_types.h"
#include "ast_factory_test_reflection.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler::test
{
    struct AstCategoryTestDescriptor
    {
        std::string category_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstCategoryTestDescriptor& desc)
        {
            return os << desc.category_name;
        }
    };

    template <typename Category>
    inline void test_single_factory_category()
    {
        constexpr std::array<size_t, 3> counts{0, 1, 4};
        for (size_t count : counts)
        {
            auto vec = sample_nodes<Category>(count, 0);
            EXPECT_EQ(vec.size(), count);
            for (const auto& item : vec)
            {
                ASSERT_NE(item, nullptr);
                EXPECT_TRUE(item->is_valid());
            }
        }

        auto single = sample_node<Category>(0);
        ASSERT_NE(single, nullptr);
        EXPECT_TRUE(single->is_valid());

        valuascript::shared::tuple_for_each_type<category_node_types_t<Category>>([]<typename ConcreteNodeT>()
        {
            auto sampled = sample_node_by_kind<Category>(ConcreteNodeT::KIND, 0);
            ASSERT_NE(sampled, nullptr);
            EXPECT_EQ(sampled->kind, ConcreteNodeT::KIND);
            EXPECT_TRUE(sampled->is_valid());

            auto as_node = sample_node_as<ConcreteNodeT>(0);
            const auto& ref = unwrap_node(as_node);
            EXPECT_EQ(ref.kind, ConcreteNodeT::KIND);
            EXPECT_TRUE(ref.is_valid());
        });
    }

    template <typename Category>
    AstCategoryTestDescriptor make_ast_category_test_descriptor()
    {
        return AstCategoryTestDescriptor{
            .category_name = std::string(category_name_v<Category>),
            .run_test = []() {
                test_single_factory_category<Category>();
            }
        };
    }

    template <typename Tuple>
    struct AstCategoryTestDescriptorCollector;

    template <typename... Categories>
    struct AstCategoryTestDescriptorCollector<std::tuple<Categories...>>
    {
        static std::vector<AstCategoryTestDescriptor> collect()
        {
            return { make_ast_category_test_descriptor<Categories>()... };
        }
    };

    inline std::vector<AstCategoryTestDescriptor> get_all_ast_category_test_descriptors()
    {
        return AstCategoryTestDescriptorCollector<AstCategoryTypes>::collect();
    }

    class AstFactoryCategoryParameterizedTest : public testing::TestWithParam<AstCategoryTestDescriptor>
    {
    };

    TEST_P(AstFactoryCategoryParameterizedTest, CategorySamplingAndInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing AST Category Factory Generation for: " + descriptor.category_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstCategoryTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstCategoryTestDescriptor>& info) const
        {
            return info.param.category_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstCategories,
        AstFactoryCategoryParameterizedTest,
        testing::ValuesIn(get_all_ast_category_test_descriptors()),
        AstCategoryTestNameGenerator{}
    );
}
