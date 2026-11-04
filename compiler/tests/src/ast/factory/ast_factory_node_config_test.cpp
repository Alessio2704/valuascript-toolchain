#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"

namespace valuascript::compiler::test
{
    template <typename T>
    auto create_test_node(int depth, const AstFactoryConfig& cfg)
    {
        if constexpr (requires { create_sample<T>(depth, cfg)->is_valid(); })
        {
            return create_sample<T>(depth, cfg);
        }
        else
        {
            return std::make_unique<T>(create_sample<T>(depth, cfg));
        }
    }

    template <typename NodeT, size_t Index>
    void test_single_config_member()
    {
        constexpr auto member_ptr = std::get<Index>(NodeConfig<NodeT>::members);
        using DescriptorT = std::remove_cvref_t<decltype(std::declval<NodeConfig<NodeT>>().*member_ptr)>;

        if constexpr (requires { typename DescriptorT::element_type; })
        {
            for (size_t count : {size_t{0}, size_t{1}, size_t{5}})
            {
                AstFactoryConfig cfg{};
                (cfg.get<NodeT>().*member_ptr).count = count;
                auto node = create_test_node<NodeT>(0, cfg);
                ASSERT_NE(node, nullptr);
                EXPECT_EQ((node.get()->*DescriptorT::member_ptr).size(), count);
                EXPECT_TRUE(node->is_valid());
            }
        }
        else if constexpr (requires { (std::declval<DescriptorT>().prefix); })
        {
            AstFactoryConfig cfg{};
            (cfg.get<NodeT>().*member_ptr).prefix = "custom_pfx_";
            auto node = create_test_node<NodeT>(0, cfg);
            ASSERT_NE(node, nullptr);
            EXPECT_EQ((node.get()->*DescriptorT::member_ptr).value.rfind("custom_pfx_", 0), 0);
            EXPECT_TRUE(node->is_valid());
        }
        else if constexpr (requires { (std::declval<DescriptorT>().op); })
        {
            TokenType initial_op = (NodeConfig<NodeT>{}.*member_ptr).op;
            TokenType test_op = initial_op == TokenType::Plus ? TokenType::Minus : TokenType::Plus;
            AstFactoryConfig cfg{};
            (cfg.get<NodeT>().*member_ptr).op = test_op;
            auto node = create_test_node<NodeT>(0, cfg);
            ASSERT_NE(node, nullptr);
            EXPECT_EQ((node.get()->*DescriptorT::member_ptr), test_op);
            EXPECT_TRUE(node->is_valid());
        }
        else if constexpr (requires { (std::declval<DescriptorT>().value); } &&
                           std::is_same_v<std::remove_cvref_t<decltype(std::declval<DescriptorT>().value)>, std::string>)
        {
            AstFactoryConfig cfg{};
            (cfg.get<NodeT>().*member_ptr).value = "custom_string_val";
            auto node = create_test_node<NodeT>(0, cfg);
            ASSERT_NE(node, nullptr);
            EXPECT_EQ((node.get()->*DescriptorT::member_ptr), "custom_string_val");
            EXPECT_TRUE(node->is_valid());
        }
        else if constexpr (requires { (std::declval<DescriptorT>().value); } &&
                           std::is_same_v<std::remove_cvref_t<decltype(std::declval<DescriptorT>().value)>, bool>)
        {
            for (bool val : {false, true})
            {
                AstFactoryConfig cfg{};
                (cfg.get<NodeT>().*member_ptr).value = val;
                auto node = create_test_node<NodeT>(0, cfg);
                ASSERT_NE(node, nullptr);
                EXPECT_EQ((node.get()->*DescriptorT::member_ptr), val);
                EXPECT_TRUE(node->is_valid());
            }
        }
        else if constexpr (requires { typename DescriptorT::value_type; })
        {
            {
                AstFactoryConfig cfg{};
                (cfg.get<NodeT>().*member_ptr).value = std::nullopt;
                auto node = create_test_node<NodeT>(0, cfg);
                ASSERT_NE(node, nullptr);
                EXPECT_FALSE((node.get()->*DescriptorT::member_ptr).has_value());
                EXPECT_TRUE(node->is_valid());
            }
            {
                AstFactoryConfig cfg{};
                if constexpr (std::is_same_v<typename DescriptorT::value_type, bool>)
                {
                    (cfg.get<NodeT>().*member_ptr).value = true;
                }
                else
                {
                    (cfg.get<NodeT>().*member_ptr).value = typename DescriptorT::value_type{};
                }
                auto node = create_test_node<NodeT>(0, cfg);
                ASSERT_NE(node, nullptr);
                EXPECT_TRUE((node.get()->*DescriptorT::member_ptr).has_value());
                EXPECT_TRUE(node->is_valid());
            }
        }
    }

    template <typename NodeT, size_t... Is>
    void test_all_config_members_helper(std::index_sequence<Is...>)
    {
        (test_single_config_member<NodeT, Is>(), ...);
    }

    template <typename NodeT>
    void test_node_config_field_mutations()
    {
        constexpr size_t N = std::tuple_size_v<decltype(NodeConfig<NodeT>::members)>;
        test_all_config_members_helper<NodeT>(std::make_index_sequence<N>{});
    }

    struct AstNodeConfigTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstNodeConfigTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    AstNodeConfigTestDescriptor make_ast_node_config_test_descriptor()
    {
        return AstNodeConfigTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []()
            {
                test_node_config_field_mutations<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstNodeConfigTestDescriptorCollector;

    template <typename... Types>
    struct AstNodeConfigTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstNodeConfigTestDescriptor> collect()
        {
            return { make_ast_node_config_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstNodeConfigTestDescriptor> get_all_ast_node_config_test_descriptors()
    {
        return AstNodeConfigTestDescriptorCollector<AllAstNodeTypes>::collect();
    }

    class AstNodeConfigParameterizedTest : public testing::TestWithParam<AstNodeConfigTestDescriptor>
    {
    };

    TEST_P(AstNodeConfigParameterizedTest, MutatingConfigFieldsAltersOutputNodeCorrectly)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Config Mutability for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstNodeConfigTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstNodeConfigTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstNodeConfigParameterizedTest,
        testing::ValuesIn(get_all_ast_node_config_test_descriptors()),
        AstNodeConfigTestNameGenerator{}
    );
}
