#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <memory>
#include <concepts>

#include "ast/walker/ast_walker.h"
#include "ast/walker/ast_walker_concepts.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/factory/ast_factory.h"
#include "ast/utils/ast_cast.h"

namespace valuascript::compiler
{
    template <typename T>
    concept IsAstChildMember =
        IsUniquePtrOfAstNode<T> ||
        IsVectorOfUniquePtrOfAstNode<T> ||
        IsInnerAstNode<T> ||
        IsVectorOfInnerAstNode<T> ||
        IsOptionalAstFieldOfAstNode<T> ||
        IsStdOptionalOfAstNode<T>;

    struct AstChildMemberDescriptor
    {
        std::string node_type_name;
        size_t member_index = 0;
        const AstNode* child_address = nullptr;
        std::string child_kind_name;
    };

    class AstWalkerValidationResult
    {
    public:
        AstWalkerValidationResult() = default;

        explicit AstWalkerValidationResult(std::vector<AstChildMemberDescriptor> unvisited)
            : unvisited_members_(std::move(unvisited))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept
        {
            return unvisited_members_.empty();
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return is_valid();
        }

        [[nodiscard]] const std::vector<AstChildMemberDescriptor>& unvisited_members() const noexcept
        {
            return unvisited_members_;
        }

        [[nodiscard]] bool has_unvisited_child(const AstNode* child) const noexcept
        {
            for (const auto& item : unvisited_members_)
            {
                if (item.child_address == child)
                {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] bool has_unvisited_member_index(size_t member_index) const noexcept
        {
            for (const auto& item : unvisited_members_)
            {
                if (item.member_index == member_index)
                {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] std::string error_message() const
        {
            if (is_valid())
            {
                return "AstWalker validation succeeded: all child properties visited.";
            }

            std::ostringstream ss;
            ss << "AstWalker validation failed: " << unvisited_members_.size() << " child node(s) were omitted during traversal:\n";
            for (size_t i = 0; i < unvisited_members_.size(); ++i)
            {
                const auto& item = unvisited_members_[i];
                ss << "  [" << i + 1 << "] Node '" << item.node_type_name
                   << "' member #" << item.member_index
                   << " -> Child (" << item.child_kind_name
                   << " at " << static_cast<const void*>(item.child_address) << ")\n";
            }
            return ss.str();
        }

    private:
        std::vector<AstChildMemberDescriptor> unvisited_members_;
    };

    class AstWalkerValidator
    {
    private:
        class TraversalRecordingWalker : public ConstAstWalker
        {
        public:
            using ConstAstWalker::enter;
            using ConstAstWalker::leave;

            std::unordered_set<const AstNode*> visited_nodes;

            TraversalAction enter_node(const AstNode& node) override
            {
                visited_nodes.insert(&node);
                return TraversalAction::Continue;
            }
        };

        template <typename T>
        static const auto& unwrap_sample(const T& node)
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

    public:
        template <typename NodeT>
        static std::vector<AstChildMemberDescriptor> collect_expected_children(const NodeT& sample)
        {
            std::vector<AstChildMemberDescriptor> result;
            const auto& target = unwrap_sample(sample);
            std::string_view node_name = get_ast_node_name<std::remove_cvref_t<decltype(target)>>();

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<std::remove_cvref_t<decltype(target)>>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ([&] {
                    [[maybe_unused]] const auto& member = target.*std::get<Is>(AstNodeSchema<std::remove_cvref_t<decltype(target)>>::members);
                    using MemberT = std::remove_cvref_t<decltype(member)>;

                    if constexpr (IsUniquePtrOfAstNode<MemberT>)
                    {
                        if (member)
                        {
                            result.push_back(AstChildMemberDescriptor{
                                .node_type_name = std::string(node_name),
                                .member_index = Is,
                                .child_address = member.get(),
                                .child_kind_name = std::string(to_string(member->kind))
                            });
                        }
                    }
                    else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberT>)
                    {
                        for (const auto& item : member)
                        {
                            if (item)
                            {
                                result.push_back(AstChildMemberDescriptor{
                                    .node_type_name = std::string(node_name),
                                    .member_index = Is,
                                    .child_address = item.get(),
                                    .child_kind_name = std::string(to_string(item->kind))
                                });
                            }
                        }
                    }
                    else if constexpr (IsInnerAstNode<MemberT>)
                    {
                        result.push_back(AstChildMemberDescriptor{
                            .node_type_name = std::string(node_name),
                            .member_index = Is,
                            .child_address = &member,
                            .child_kind_name = std::string(to_string(member.kind))
                        });
                    }
                    else if constexpr (IsVectorOfInnerAstNode<MemberT>)
                    {
                        for (const auto& item : member)
                        {
                            result.push_back(AstChildMemberDescriptor{
                                .node_type_name = std::string(node_name),
                                .member_index = Is,
                                .child_address = &item,
                                .child_kind_name = std::string(to_string(item.kind))
                            });
                        }
                    }
                    else if constexpr (IsOptionalAstFieldOfAstNode<MemberT>)
                    {
                        if (member.has_value())
                        {
                            using ValT = typename MemberT::value_type;
                            if constexpr (is_unique_ptr_v<ValT>)
                            {
                                if (member.get())
                                {
                                    result.push_back(AstChildMemberDescriptor{
                                        .node_type_name = std::string(node_name),
                                        .member_index = Is,
                                        .child_address = member.get(),
                                        .child_kind_name = std::string(to_string(member->kind))
                                    });
                                }
                            }
                            else
                            {
                                result.push_back(AstChildMemberDescriptor{
                                    .node_type_name = std::string(node_name),
                                    .member_index = Is,
                                    .child_address = &(*member),
                                    .child_kind_name = std::string(to_string((*member).kind))
                                });
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
                                if (*member)
                                {
                                    result.push_back(AstChildMemberDescriptor{
                                        .node_type_name = std::string(node_name),
                                        .member_index = Is,
                                        .child_address = (*member).get(),
                                        .child_kind_name = std::string(to_string((*member)->kind))
                                    });
                                }
                            }
                            else
                            {
                                result.push_back(AstChildMemberDescriptor{
                                    .node_type_name = std::string(node_name),
                                    .member_index = Is,
                                    .child_address = &(*member),
                                    .child_kind_name = std::string(to_string((*member).kind))
                                });
                            }
                        }
                    }
                }(), ...);
            }(std::make_index_sequence<N>{});

            return result;
        }

        template <typename NodeT>
        static AstWalkerValidationResult validate_default_walker(int depth = 2)
        {
            auto sample = create_sample<NodeT>(depth);
            auto expected = collect_expected_children(sample);

            TraversalRecordingWalker recorder;
            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                recorder.walk(sample);
            }
            else
            {
                recorder.walk(sample.get());
            }

            std::vector<AstChildMemberDescriptor> unvisited;
            for (const auto& desc : expected)
            {
                if (!recorder.visited_nodes.contains(desc.child_address))
                {
                    unvisited.push_back(desc);
                }
            }

            return AstWalkerValidationResult(std::move(unvisited));
        }

        template <typename WalkerT, typename SampleT>
        static AstWalkerValidationResult validate_visited_against_expected(
            const std::vector<AstChildMemberDescriptor>& expected,
            const std::unordered_set<const AstNode*>& visited)
        {
            std::vector<AstChildMemberDescriptor> unvisited;
            for (const auto& desc : expected)
            {
                if (!visited.contains(desc.child_address))
                {
                    unvisited.push_back(desc);
                }
            }
            return AstWalkerValidationResult(std::move(unvisited));
        }
    };
}
