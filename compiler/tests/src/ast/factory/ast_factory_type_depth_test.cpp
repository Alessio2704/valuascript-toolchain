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
#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/categories/ast_type_annotation_types.h"

namespace valuascript::compiler::test
{
    template <typename Visitor>
    void visit_child_type_annotations(const auto& type_node, Visitor&& visitor)
    {
        for_each_ast_member(type_node, [&](const auto& member)
        {
            using MemberType = std::remove_cvref_t<decltype(member)>;
            if constexpr (std::same_as<MemberType, TypeAnnPtr>)
            {
                if (member != nullptr)
                {
                    visitor(*member);
                }
            }
            else if constexpr (std::same_as<MemberType, std::vector<TypeAnnPtr>>)
            {
                for (const auto& elem : member)
                {
                    if (elem != nullptr)
                    {
                        visitor(*elem);
                    }
                }
            }
        });
    }

    struct AstTypeDepthTestDescriptor
    {
        std::string type_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstTypeDepthTestDescriptor& desc)
        {
            return os << desc.type_name;
        }
    };

    template <typename T>
    inline void test_single_type_depth_handling()
    {
        constexpr std::array<int, 5> depths{0, 1, 2, 3, 4};
        for (int depth : depths)
        {
            auto type_node = create_sample<T>(depth);
            ASSERT_NE(type_node, nullptr);
            EXPECT_EQ(type_node->kind, T::KIND);
            EXPECT_TRUE(type_node->is_valid());

            const size_t expected_col_start = (static_cast<size_t>(depth) * 4) + 1;
            const size_t expected_col_end = (static_cast<size_t>(depth) * 4) + 13;
            EXPECT_EQ(type_node->span.column_start, expected_col_start);
            EXPECT_EQ(type_node->span.column_end, expected_col_end);

            visit_child_type_annotations(*type_node, [&](const TypeAnnotation& child)
            {
                EXPECT_GE(child.span.column_start, ((static_cast<size_t>(depth) + 1) * 4) + 1);
                EXPECT_TRUE(child.is_valid());
            });
        }

        AstFactoryConfig cutoff_cfg{
            .general = {
                .max_type_depth = 2,
                .type_kind = TypeAnnotationKind::from<T>()
            }
        };

        auto at_0 = sample_type(0, cutoff_cfg);
        ASSERT_NE(at_0, nullptr);
        EXPECT_EQ(at_0->kind, T::KIND);

        auto at_1 = sample_type(1, cutoff_cfg);
        ASSERT_NE(at_1, nullptr);
        EXPECT_EQ(at_1->kind, T::KIND);

        auto at_2 = sample_type(2, cutoff_cfg);
        ASSERT_NE(at_2, nullptr);
        EXPECT_EQ(at_2->kind, AstKind::TypeAnnotation);
        auto* leaf2 = dynamic_cast<TypeAnnotation*>(at_2.get());
        ASSERT_NE(leaf2, nullptr);
        EXPECT_EQ(leaf2->name.value.rfind("LeafType_", 0), 0);
        EXPECT_TRUE(leaf2->generic_args.empty());

        auto at_3 = sample_type(3, cutoff_cfg);
        ASSERT_NE(at_3, nullptr);
        EXPECT_EQ(at_3->kind, AstKind::TypeAnnotation);

        AstFactoryConfig zero_depth_cfg{
            .general = {
                .max_type_depth = 0,
                .type_kind = TypeAnnotationKind::from<T>()
            }
        };

        auto at_zero = sample_type(0, zero_depth_cfg);
        ASSERT_NE(at_zero, nullptr);
        EXPECT_EQ(at_zero->kind, AstKind::TypeAnnotation);
        auto* leaf_zero = dynamic_cast<TypeAnnotation*>(at_zero.get());
        ASSERT_NE(leaf_zero, nullptr);
        EXPECT_EQ(leaf_zero->name.value.rfind("LeafType_", 0), 0);
    }

    template <typename T>
    AstTypeDepthTestDescriptor make_ast_type_depth_test_descriptor()
    {
        return AstTypeDepthTestDescriptor{
            .type_name = std::string(get_ast_node_name<T>()),
            .run_test = []() {
                test_single_type_depth_handling<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstTypeDepthTestDescriptorCollector;

    template <typename... Types>
    struct AstTypeDepthTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstTypeDepthTestDescriptor> collect()
        {
            return { make_ast_type_depth_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstTypeDepthTestDescriptor> get_all_ast_type_depth_test_descriptors()
    {
        return AstTypeDepthTestDescriptorCollector<AllTypeAnnotationNodeTypes>::collect();
    }

    class AstFactoryTypeDepthParameterizedTest : public testing::TestWithParam<AstTypeDepthTestDescriptor>
    {
    };

    TEST_P(AstFactoryTypeDepthParameterizedTest, TypeDepthAndGenericNestingInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Type Depth & Generic Nesting for: " + descriptor.type_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstTypeTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstTypeDepthTestDescriptor>& info) const
        {
            return info.param.type_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllTypeDepthTests,
        AstFactoryTypeDepthParameterizedTest,
        testing::ValuesIn(get_all_ast_type_depth_test_descriptors()),
        AstTypeTestNameGenerator{}
    );
}
