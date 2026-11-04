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

namespace valuascript::compiler::test
{
    struct AstConstConfigTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstConstConfigTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    inline void test_const_config_access()
    {
        const AstFactoryConfig const_cfg{};

        const auto& node_cfg = const_cfg.get<T>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(node_cfg)>, NodeConfig<T>>);

        auto sample = create_sample<T>(0, const_cfg);
        if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
        {
            ASSERT_NE(sample, nullptr);
            EXPECT_EQ(sample->kind, T::KIND);
            EXPECT_TRUE(sample->is_valid());
        }
        else
        {
            EXPECT_EQ(sample.kind, T::KIND);
            EXPECT_TRUE(sample.is_valid());
        }

        auto sampled_node = sample_node<T>(0, const_cfg);
        if constexpr (std::same_as<decltype(sampled_node), std::unique_ptr<T>>)
        {
            ASSERT_NE(sampled_node, nullptr);
            EXPECT_EQ(sampled_node->kind, T::KIND);
            EXPECT_TRUE(sampled_node->is_valid());
        }
        else
        {
            EXPECT_EQ(sampled_node.kind, T::KIND);
            EXPECT_TRUE(sampled_node.is_valid());
        }
    }

    template <typename T>
    AstConstConfigTestDescriptor make_ast_const_config_test_descriptor()
    {
        return AstConstConfigTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []()
            {
                test_const_config_access<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstConstConfigTestDescriptorCollector;

    template <typename... Types>
    struct AstConstConfigTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstConstConfigTestDescriptor> collect()
        {
            return { make_ast_const_config_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstConstConfigTestDescriptor> get_all_ast_const_config_test_descriptors()
    {
        return AstConstConfigTestDescriptorCollector<AllAstNodeTypes>::collect();
    }

    class AstConstConfigParameterizedTest : public testing::TestWithParam<AstConstConfigTestDescriptor>
    {
    };

    TEST_P(AstConstConfigParameterizedTest, ConstAccessorAndFactorySampling)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Const AstFactoryConfig for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstConstConfigTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstConstConfigTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstConstConfigParameterizedTest,
        testing::ValuesIn(get_all_ast_const_config_test_descriptors()),
        AstConstConfigTestNameGenerator{}
    );

    TEST(AstConstConfigTest, GeneralConstSamplingMethods)
    {
        const AstFactoryConfig const_cfg{};

        auto expr = sample_expr(0, const_cfg);
        ASSERT_NE(expr, nullptr);
        EXPECT_TRUE(expr->is_valid());

        auto stmt = sample_stmt(0, const_cfg);
        ASSERT_NE(stmt, nullptr);
        EXPECT_TRUE(stmt->is_valid());

        auto type_node = sample_type(0, const_cfg);
        ASSERT_NE(type_node, nullptr);
        EXPECT_TRUE(type_node->is_valid());

        auto prog = create_sample_program(0, const_cfg);
        ASSERT_NE(prog, nullptr);
        EXPECT_TRUE(prog->is_valid());
    }
}
