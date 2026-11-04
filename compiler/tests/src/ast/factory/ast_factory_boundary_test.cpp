#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>
#include <type_traits>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast_factory_test_reflection.h"

namespace valuascript::compiler::test
{
    struct AstBoundaryTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstBoundaryTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    inline void test_node_boundary_conditions()
    {
        for (int depth : {0, 1, 5, 10, 20})
        {
            auto sample = create_sample<T>(depth);
            const auto& node_ref = unwrap_node(sample);
            EXPECT_TRUE(node_ref.is_valid());

            const size_t expected_col_start = (static_cast<size_t>(depth) * 4) + 1;
            const size_t expected_col_end = (static_cast<size_t>(depth) * 4) + 13;
            EXPECT_EQ(node_ref.span.column_start, expected_col_start);
            EXPECT_EQ(node_ref.span.column_end, expected_col_end);
            EXPECT_TRUE(node_ref.span.is_valid());
        }

        AstFactoryConfig zero_cfg{};
        apply_zero_to_node_config<T>(zero_cfg);
        auto zero_sample = create_sample<T>(0, zero_cfg);
        const auto& zero_ref = unwrap_node(zero_sample);
        EXPECT_TRUE(zero_ref.is_valid());
        verify_vector_members_empty(zero_ref);

        constexpr size_t scaled_count = 5;
        AstFactoryConfig large_cfg{};
        apply_count_to_node_config<T>(large_cfg, scaled_count);
        auto large_sample = create_sample<T>(0, large_cfg);
        const auto& large_ref = unwrap_node(large_sample);
        EXPECT_TRUE(large_ref.is_valid());
        verify_vector_members_size(large_ref, scaled_count);

        std::string long_str(300, 'X');
        AstFactoryConfig str_cfg{};
        apply_string_to_node_config<T>(str_cfg, long_str);
        auto str_sample = create_sample<T>(0, str_cfg);
        const auto& str_ref = unwrap_node(str_sample);
        EXPECT_TRUE(str_ref.is_valid());
    }

    template <typename T>
    AstBoundaryTestDescriptor make_ast_boundary_test_descriptor()
    {
        return AstBoundaryTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []()
            {
                test_node_boundary_conditions<T>();
            }
        };
    }

    inline std::vector<AstBoundaryTestDescriptor> get_all_ast_boundary_test_descriptors()
    {
        return collect_test_descriptors<AstBoundaryTestDescriptor, AllAstNodeTypes>([]<typename T>()
        {
            return make_ast_boundary_test_descriptor<T>();
        });
    }

    class AstFactoryBoundaryParameterizedTest : public testing::TestWithParam<AstBoundaryTestDescriptor>
    {
    };

    TEST_P(AstFactoryBoundaryParameterizedTest, MetadataDrivenBoundaryAndScalingInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Metadata Boundary Conditions for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstBoundaryTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstBoundaryTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstFactoryBoundaryParameterizedTest,
        testing::ValuesIn(get_all_ast_boundary_test_descriptors()),
        AstBoundaryTestNameGenerator{}
    );

    TEST(AstFactoryBoundaryTest, NonPositiveCutoffDepthsTerminateRecursionImmediately)
    {
        for (int non_pos_depth : {0, -1, -5})
        {
            AstFactoryConfig cfg{
                .general = {
                    .max_expression_depth = non_pos_depth,
                    .max_type_depth = non_pos_depth,
                    .expression_kind = ExpressionKind::from<StringLiteral>()
                }
            };

            auto expr = sample_expr(0, cfg);
            ASSERT_NE(expr, nullptr);
            EXPECT_EQ(expr->kind, AstKind::NumberLiteral);

            auto type_node = sample_type(0, cfg);
            ASSERT_NE(type_node, nullptr);
            EXPECT_EQ(type_node->kind, AstKind::TypeAnnotation);
            auto* leaf = dynamic_cast<TypeAnnotation*>(type_node.get());
            ASSERT_NE(leaf, nullptr);
            EXPECT_EQ(leaf->name.value.rfind("LeafType_", 0), 0);
        }
    }
}
