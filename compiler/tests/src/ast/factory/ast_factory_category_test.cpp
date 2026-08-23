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
    }

    template <typename Category>
    AstCategoryTestDescriptor make_ast_category_test_descriptor()
    {
        std::string name;
        if constexpr (std::same_as<Category, Statement>)
        {
            name = "Statement";
        }
        else if constexpr (std::same_as<Category, Expression>)
        {
            name = "Expression";
        }
        else if constexpr (std::same_as<Category, TypeAnnotation>)
        {
            name = "TypeAnnotation";
        }
        else
        {
            name = "UnknownCategory";
        }

        return AstCategoryTestDescriptor{
            .category_name = std::move(name),
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
