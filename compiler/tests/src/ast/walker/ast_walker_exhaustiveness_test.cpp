#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <utility>

#include "ast_walker_test_helper.h"
#include "ast/walker/ast_walker_validator.h"

namespace valuascript::compiler::test
{
    template <typename NodeT>
    struct AstWalkerPositiveExhaustivenessTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    auto result = AstWalkerValidator::validate_default_walker<NodeT>(2);
                    EXPECT_TRUE(result.is_valid()) << result.error_message();
                }
            };
        }
    };

    class AstWalkerPositiveExhaustivenessParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerPositiveExhaustivenessParameterizedTest, DefaultWalkerVisitsEveryChildProperty)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Positive Exhaustiveness for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerPositiveExhaustivenessParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerPositiveExhaustivenessTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );

    template <typename TargetNodeT, size_t OmittedIndex>
    class SelectiveOmissionWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;
        using AstWalker::walk_children;

        std::unordered_set<const AstNode*> visited_nodes;

        TraversalAction enter_node(AstNode& node) override
        {
            visited_nodes.insert(&node);
            return TraversalAction::Continue;
        }

        void walk_children(TargetNodeT& node) override
        {
            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<TargetNodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ([&] {
                    if constexpr (Is != OmittedIndex)
                    {
                        [[maybe_unused]] auto& member = node.*std::get<Is>(AstNodeSchema<TargetNodeT>::members);
                        using MemberT = std::remove_cvref_t<decltype(member)>;

                        if constexpr (IsUniquePtrOfAstNode<MemberT>)
                        {
                            if (member) walk(member.get());
                        }
                        else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberT>)
                        {
                            for (auto& item : member)
                            {
                                if (item) walk(item.get());
                            }
                        }
                        else if constexpr (IsInnerAstNode<MemberT>)
                        {
                            walk(member);
                        }
                        else if constexpr (IsVectorOfInnerAstNode<MemberT>)
                        {
                            for (auto& item : member)
                            {
                                walk(item);
                            }
                        }
                        else if constexpr (IsOptionalAstFieldOfAstNode<MemberT>)
                        {
                            if (member.has_value())
                            {
                                using ValT = typename MemberT::value_type;
                                if constexpr (is_unique_ptr_v<ValT>)
                                {
                                    if (member.get()) walk(member.get());
                                }
                                else
                                {
                                    walk(*member);
                                }
                            }
                        }
                        else if constexpr (IsStdOptionalOfAstNode<MemberT>)
                        {
                            if (member.has_value())
                            {
                                using ValT = typename MemberT::value_type;
                                if constexpr (is_unique_ptr_v<ValT>)
                                {
                                    if (*member) walk((*member).get());
                                }
                                else
                                {
                                    walk(*member);
                                }
                            }
                        }
                    }
                }(), ...);
            }(std::make_index_sequence<N>{});
        }
    };

    template <typename NodeT>
    inline void run_negative_sweep_for_node()
    {
        constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;

        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ([&] {
                using MemberPtrT = std::tuple_element_t<Is, decltype(AstNodeSchema<NodeT>::members)>;
                using MemberRawT = std::remove_cvref_t<decltype(std::declval<NodeT>().*std::declval<MemberPtrT>())>;

                if constexpr (IsAstChildMember<MemberRawT>)
                {
                    auto sample = create_sample<NodeT>(2);
                    auto expected = AstWalkerValidator::collect_expected_children(sample);

                    bool expected_has_omitted_member = false;
                    for (const auto& desc : expected)
                    {
                        if (desc.member_index == Is)
                        {
                            expected_has_omitted_member = true;
                            break;
                        }
                    }

                    if (expected_has_omitted_member)
                    {
                        SelectiveOmissionWalker<NodeT, Is> omission_walker;
                        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
                        {
                            omission_walker.walk(sample);
                        }
                        else
                        {
                            ASSERT_NE(sample, nullptr);
                            omission_walker.walk(sample.get());
                        }

                        auto result = AstWalkerValidator::validate_visited_against_expected<SelectiveOmissionWalker<NodeT, Is>, decltype(sample)>(
                            expected, omission_walker.visited_nodes);

                        EXPECT_FALSE(result.is_valid())
                            << "Failed to catch omission of member index " << Is
                            << " in node " << get_ast_node_name<NodeT>();
                        EXPECT_TRUE(result.has_unvisited_member_index(Is))
                            << "Omitted member index " << Is << " was not flagged in validation result!";
                    }
                }
            }(), ...);
        }(std::make_index_sequence<N>{});
    }

    template <typename NodeT>
    struct AstWalkerNegativeSweepTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_negative_sweep_for_node<NodeT>();
                }
            };
        }
    };

    class AstWalkerNegativeSweepParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerNegativeSweepParameterizedTest, DetectsEveryPossibleChildPropertyOmission)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Negative Property Omission Sweep for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerNegativeSweepParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerNegativeSweepTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );
}
