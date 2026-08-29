#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <tuple>
#include <type_traits>
#include <memory>
#include <gtest/gtest.h>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_schema.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler::test
{
    template <typename T>
    struct is_std_vector : std::false_type {};

    template <typename T, typename Alloc>
    struct is_std_vector<std::vector<T, Alloc>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_std_vector_v = is_std_vector<std::remove_cvref_t<T>>::value;

    template <typename T>
    inline const auto& unwrap_node(const T& node)
    {
        if constexpr (requires { node.get(); })
        {
            return *node;
        }
        else
        {
            return node;
        }
    }

    template <typename NodeT>
    inline void apply_zero_to_node_config(AstFactoryConfig& cfg)
    {
        constexpr size_t N = std::tuple_size_v<decltype(NodeConfig<NodeT>::members)>;
        [&]<size_t... Is>(std::index_sequence<Is...>)
        {
            ([&]
            {
                constexpr auto member_ptr = std::get<Is>(NodeConfig<NodeT>::members);
                using DescriptorT = std::remove_cvref_t<decltype(std::declval<NodeConfig<NodeT>>().*member_ptr)>;

                if constexpr (requires { (cfg.get<NodeT>().*member_ptr).count; })
                {
                    (cfg.get<NodeT>().*member_ptr).count = 0;
                }
                else if constexpr (requires { (cfg.get<NodeT>().*member_ptr).value; })
                {
                    if constexpr (requires { typename DescriptorT::value_type; })
                    {
                        (cfg.get<NodeT>().*member_ptr).value = std::nullopt;
                    }
                }
            }(), ...);
        }(std::make_index_sequence<N>{});
    }

    template <typename NodeT>
    inline void apply_count_to_node_config(AstFactoryConfig& cfg, size_t count)
    {
        constexpr size_t N = std::tuple_size_v<decltype(NodeConfig<NodeT>::members)>;
        [&]<size_t... Is>(std::index_sequence<Is...>)
        {
            ([&]
            {
                constexpr auto member_ptr = std::get<Is>(NodeConfig<NodeT>::members);
                if constexpr (requires { (cfg.get<NodeT>().*member_ptr).count; })
                {
                    (cfg.get<NodeT>().*member_ptr).count = count;
                }
            }(), ...);
        }(std::make_index_sequence<N>{});
    }

    template <typename NodeT>
    inline void apply_string_to_node_config(AstFactoryConfig& cfg, std::string_view str)
    {
        constexpr size_t N = std::tuple_size_v<decltype(NodeConfig<NodeT>::members)>;
        [&]<size_t... Is>(std::index_sequence<Is...>)
        {
            ([&]
            {
                constexpr auto member_ptr = std::get<Is>(NodeConfig<NodeT>::members);
                using DescriptorT = std::remove_cvref_t<decltype(std::declval<NodeConfig<NodeT>>().*member_ptr)>;

                if constexpr (requires { (cfg.get<NodeT>().*member_ptr).prefix; })
                {
                    (cfg.get<NodeT>().*member_ptr).prefix = std::string(str);
                }
                else if constexpr (requires { (cfg.get<NodeT>().*member_ptr).value; })
                {
                    if constexpr (std::is_same_v<std::remove_cvref_t<decltype((cfg.get<NodeT>().*member_ptr).value)>, std::string>)
                    {
                        (cfg.get<NodeT>().*member_ptr).value = std::string(str);
                    }
                    else if constexpr (requires { typename DescriptorT::value_type; })
                    {
                        if constexpr (std::is_same_v<typename DescriptorT::value_type, std::string>)
                        {
                            (cfg.get<NodeT>().*member_ptr).value = std::string(str);
                        }
                    }
                }
            }(), ...);
        }(std::make_index_sequence<N>{});
    }

    template <typename T>
    inline void verify_vector_members_size(const T& node, size_t expected_size)
    {
        const auto& target = unwrap_node(node);
        for_each_ast_member(target, [&](const auto& member)
        {
            using MemberType = std::remove_cvref_t<decltype(member)>;
            if constexpr (is_std_vector_v<MemberType>)
            {
                EXPECT_EQ(member.size(), expected_size);
                for (const auto& item : member)
                {
                    if constexpr (requires { item->is_valid(); })
                    {
                        ASSERT_NE(item, nullptr);
                        EXPECT_TRUE(item->is_valid());
                    }
                    else if constexpr (requires { item.is_valid(); })
                    {
                        EXPECT_TRUE(item.is_valid());
                    }
                }
            }
        });
    }

    template <typename T>
    inline void verify_vector_members_empty(const T& node)
    {
        const auto& target = unwrap_node(node);
        for_each_ast_member(target, [&](const auto& member)
        {
            using MemberType = std::remove_cvref_t<decltype(member)>;
            if constexpr (is_std_vector_v<MemberType>)
            {
                EXPECT_TRUE(member.empty());
            }
        });
    }

    template <typename DescriptorT, typename TupleT, typename MakeDescFunc>
    inline std::vector<DescriptorT> collect_test_descriptors(MakeDescFunc&& make_desc)
    {
        std::vector<DescriptorT> descriptors;
        valuascript::shared::tuple_for_each_type<TupleT>([&]<typename T>()
        {
            descriptors.push_back(make_desc.template operator()<T>());
        });
        return descriptors;
    }
}
