#pragma once

#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/metadata/ast_node_registry.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    template <typename ClassT, typename Tuple>
    struct ExtractConfigFieldTypesHelper;

    template <typename ClassT, typename... MemberPtrs>
    struct ExtractConfigFieldTypesHelper<ClassT, std::tuple<MemberPtrs...>>
    {
        using type = std::tuple<std::remove_cvref_t<decltype(std::declval<ClassT>().*std::declval<MemberPtrs>())>...>;
    };

    template <typename ClassT, typename Tuple>
    using ExtractConfigFieldTypes = ExtractConfigFieldTypesHelper<ClassT, std::remove_cvref_t<Tuple>>;

    template <auto MemberPtr, typename NodeT>
    struct RequiredDescriptorForMember;

    template <typename NodeT, typename ClassT, typename ElemT, std::vector<ElemT> ClassT::*MemberPtr>
    struct RequiredDescriptorForMember<MemberPtr, NodeT>
    {
        using type = VectorFieldConfig<MemberPtr>;
    };

    template <typename NodeT, typename ClassT, NodeName ClassT::*MemberPtr>
    struct RequiredDescriptorForMember<MemberPtr, NodeT>
    {
        using type = NameFieldConfig<MemberPtr>;
    };

    template <typename NodeT, typename ClassT, TokenType ClassT::*MemberPtr>
    struct RequiredDescriptorForMember<MemberPtr, NodeT>
    {
        using type = OperatorFieldConfig<MemberPtr>;
    };

    template <typename NodeT, typename ClassT, std::string ClassT::*MemberPtr>
    struct RequiredDescriptorForMember<MemberPtr, NodeT>
    {
        using type = StringFieldConfig<MemberPtr>;
    };

    template <typename NodeT, typename ClassT, bool ClassT::*MemberPtr>
    struct RequiredDescriptorForMember<MemberPtr, NodeT>
    {
        using type = BoolFieldConfig<MemberPtr>;
    };

    template <typename NodeT, typename ClassT, typename ValT, OptionalAstField<ValT> ClassT::*MemberPtr>
    struct RequiredDescriptorForMember<MemberPtr, NodeT>
    {
        using ConfigValType = std::conditional_t<
            std::is_same_v<ValT, ExprPtr> || std::is_same_v<ValT, StmtPtr> || std::is_same_v<ValT, TypeAnnPtr>,
            bool,
            ValT
        >;
        using type = OptionalFieldConfig<MemberPtr, ConfigValType>;
    };

    template <typename NodeT, typename ConfigT, size_t Index>
    consteval bool verify_node_config_member()
    {
        constexpr auto member_ptr = std::get<Index>(AstNodeSchema<NodeT>::members);
        using MemberType = std::remove_cvref_t<decltype(std::declval<NodeT>().*member_ptr)>;

        if constexpr (requires { typename RequiredDescriptorForMember<member_ptr, NodeT>::type; } &&
            requires { ConfigT::members; })
        {
            using ExpectedDescriptor = typename RequiredDescriptorForMember<member_ptr, NodeT>::type;
            using ConfigFieldTypes = typename ExtractConfigFieldTypes<ConfigT, decltype(ConfigT::members)>::type;
            return valuascript::shared::tuple_contains_type_v<ExpectedDescriptor, ConfigFieldTypes>;
        }
        else if constexpr (std::is_same_v<MemberType, SourceSpan> ||
            std::is_same_v<MemberType, std::unique_ptr<Expression>> ||
            std::is_same_v<MemberType, std::unique_ptr<Statement>> ||
            std::is_same_v<MemberType, std::unique_ptr<TypeAnnotation>>)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename NodeT, typename ConfigT, size_t... Is>
    consteval bool verify_all_node_config_members_helper(std::index_sequence<Is...>)
    {
        return (verify_node_config_member<NodeT, ConfigT, Is>() && ...);
    }

    template <typename NodeT, typename ConfigT = NodeConfig<NodeT>>
    struct NodeConfigValidator
    {
        static consteval bool validate()
        {
            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            return verify_all_node_config_members_helper<NodeT, ConfigT>(std::make_index_sequence<N>{});
        }
    };

    template <typename NodeT, typename ConfigT = NodeConfig<NodeT>>
    concept ValidNodeConfig = NodeConfigValidator<NodeT, ConfigT>::validate();

    template <typename Tuple>
    struct AllNodeConfigsValidator;

    template <typename... Nodes>
    struct AllNodeConfigsValidator<std::tuple<Nodes...>>
    {
        static consteval bool validate()
        {
            return (NodeConfigValidator<Nodes>::validate() && ...);
        }
    };
}
