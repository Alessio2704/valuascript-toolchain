#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <type_traits>
#include <ostream>

#include "token/source_span.h"
#include "ast/core/ast_core.h"
#include "ast/clone/ast_clone.h"
#include "ast/equality/ast_equality.h"
#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/walker/ast_walker_concepts.h"

namespace valuascript::compiler::test
{
    template <typename OrigT, typename MutatedT>
    inline void assert_mutation_breaks_equality(const OrigT& orig, const MutatedT& mutated)
    {
        if constexpr (valuascript::shared::tuple_contains_type_v<OrigT, AllInnerNodeTypes>)
        {
            EXPECT_FALSE(ast_equals(orig, mutated));
            EXPECT_FALSE(ast_equals(mutated, orig));
        }
        else
        {
            EXPECT_FALSE(ast_equals(&orig, mutated.get()));
            EXPECT_FALSE(ast_equals(mutated.get(), &orig));
            EXPECT_FALSE(ast_equals(orig, *mutated));
            EXPECT_FALSE(ast_equals(*mutated, orig));
        }
    }

    template <typename OrigT, typename CloneFactory, typename Getter>
    inline void mutate_and_verify_span_subproperties(const OrigT& orig, CloneFactory&& make_clone, Getter&& get_span)
    {
        {
            auto mutated = make_clone();
            get_span(mutated).start_offset += 100;
            assert_mutation_breaks_equality(orig, mutated);
        }
        {
            auto mutated = make_clone();
            get_span(mutated).length += 10;
            assert_mutation_breaks_equality(orig, mutated);
        }
        {
            auto mutated = make_clone();
            get_span(mutated).line_start += 1;
            assert_mutation_breaks_equality(orig, mutated);
        }
        {
            auto mutated = make_clone();
            get_span(mutated).line_end += 1;
            assert_mutation_breaks_equality(orig, mutated);
        }
        {
            auto mutated = make_clone();
            get_span(mutated).column_start += 1;
            assert_mutation_breaks_equality(orig, mutated);
        }
        {
            auto mutated = make_clone();
            get_span(mutated).column_end += 1;
            assert_mutation_breaks_equality(orig, mutated);
        }
    }

    template <typename OrigT, typename CloneFactory, typename Getter>
    inline void mutate_and_verify_nodename(const OrigT& orig, CloneFactory&& make_clone, Getter&& get_name)
    {
        {
            auto mutated = make_clone();
            get_name(mutated).value += "_mutated_name_value";
            assert_mutation_breaks_equality(orig, mutated);
        }
        mutate_and_verify_span_subproperties(orig, make_clone, [&](auto& node) -> valuascript::shared::SourceSpan& {
            return get_name(node).span;
        });
    }

    template <typename ConcreteNode, typename OrigT, typename CloneFactory, typename Getter>
    inline void mutate_concrete_ast_node_members(const OrigT& orig, CloneFactory&& make_clone, Getter&& get_node)
    {
        constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<ConcreteNode>::members)>;
        [&]<size_t... Js>(std::index_sequence<Js...>) {
            ([&]() {
                using MemberRefT = decltype(std::declval<ConcreteNode&>().*std::get<Js>(AstNodeSchema<ConcreteNode>::members));
                using RawMemberT = std::remove_cvref_t<MemberRefT>;

                if constexpr (std::same_as<RawMemberT, SourceSpan> || std::same_as<RawMemberT, valuascript::shared::SourceSpan>)
                {
                    mutate_and_verify_span_subproperties(orig, make_clone, [&](auto& root) -> valuascript::shared::SourceSpan& {
                        return get_node(root).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    });
                }
                else if constexpr (std::same_as<RawMemberT, NodeName>)
                {
                    mutate_and_verify_nodename(orig, make_clone, [&](auto& root) -> NodeName& {
                        return get_node(root).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    });
                }
                else if constexpr (std::same_as<RawMemberT, std::string>)
                {
                    auto mutated = make_clone();
                    get_node(mutated).*std::get<Js>(AstNodeSchema<ConcreteNode>::members) += "_mutated_val";
                    assert_mutation_breaks_equality(orig, mutated);
                }
                else if constexpr (std::same_as<RawMemberT, bool>)
                {
                    auto mutated = make_clone();
                    auto& b = get_node(mutated).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    b = !b;
                    assert_mutation_breaks_equality(orig, mutated);
                }
                else if constexpr (std::same_as<RawMemberT, TokenType>)
                {
                    auto mutated = make_clone();
                    auto& tok = get_node(mutated).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    tok = (tok == TokenType::Plus ? TokenType::Minus : TokenType::Plus);
                    assert_mutation_breaks_equality(orig, mutated);
                }
                else if constexpr (IsUniquePtrOfAstNode<RawMemberT>)
                {
                    auto mutated = make_clone();
                    auto& child = get_node(mutated).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    if (child)
                    {
                        child->span.start_offset += 100;
                        assert_mutation_breaks_equality(orig, mutated);
                    }
                }
                else if constexpr (is_optional_ast_field_v<RawMemberT>)
                {
                    auto mutated = make_clone();
                    auto& opt = get_node(mutated).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    if (opt.has_value())
                    {
                        opt.reset();
                        assert_mutation_breaks_equality(orig, mutated);
                    }
                }
                else if constexpr (requires { typename RawMemberT::value_type; })
                {
                    auto mutated = make_clone();
                    auto& vec = get_node(mutated).*std::get<Js>(AstNodeSchema<ConcreteNode>::members);
                    if (!vec.empty())
                    {
                        vec.pop_back();
                        assert_mutation_breaks_equality(orig, mutated);
                    }
                }
            }(), ...);
        }(std::make_index_sequence<N>{});
    }

    template <typename ChildT, typename OrigT, typename CloneFactory, typename Getter>
    inline void mutate_and_verify_child_pointer(const OrigT& orig, CloneFactory&& make_clone, Getter&& get_ptr)
    {
        auto sample = make_clone();
        auto& ptr = get_ptr(sample);
        if (!ptr) return;

        if constexpr (HasAstNodeSchema<ChildT>)
        {
            mutate_concrete_ast_node_members<ChildT>(orig, make_clone, [&](auto& root) -> ChildT& {
                return *get_ptr(root);
            });
        }
        else
        {
            NodeDispatcher<AllAstNodeTypes>::dispatch(ptr->kind, [&]<typename ConcreteChild>() {
                if constexpr (std::derived_from<ConcreteChild, ChildT> && HasAstNodeSchema<ConcreteChild>)
                {
                    mutate_concrete_ast_node_members<ConcreteChild>(orig, make_clone, [&](auto& root) -> ConcreteChild& {
                        return *static_cast<ConcreteChild*>(get_ptr(root).get());
                    });
                }
            });
        }
    }

    template <typename ElemT, typename OrigT, typename CloneFactory, typename Getter>
    inline void mutate_and_verify_vector_member(const OrigT& orig, CloneFactory&& make_clone, Getter&& get_vec)
    {
        {
            auto mutated = make_clone();
            auto& vec = get_vec(mutated);
            if (!vec.empty())
            {
                vec.pop_back();
                assert_mutation_breaks_equality(orig, mutated);
            }
        }
        {
            auto sample = make_clone();
            auto& vec = get_vec(sample);
            if (!vec.empty())
            {
                if constexpr (IsUniquePtrOfAstNode<ElemT>)
                {
                    using ChildT = typename ElemT::element_type;
                    mutate_and_verify_child_pointer<ChildT>(orig, make_clone, [&](auto& root) -> auto& {
                        return get_vec(root).front();
                    });
                }
                else if constexpr (IsInnerAstNode<ElemT>)
                {
                    mutate_concrete_ast_node_members<ElemT>(orig, make_clone, [&](auto& root) -> ElemT& {
                        return get_vec(root).front();
                    });
                }
                else if constexpr (std::same_as<ElemT, NodeName>)
                {
                    mutate_and_verify_nodename(orig, make_clone, [&](auto& root) -> NodeName& {
                        return get_vec(root).front();
                    });
                }
                else if constexpr (std::same_as<ElemT, std::string>)
                {
                    auto mutated = make_clone();
                    get_vec(mutated).front() += "_mutated_vec_elem";
                    assert_mutation_breaks_equality(orig, mutated);
                }
            }
        }
    }

    template <typename ValT, typename OrigT, typename CloneFactory, typename Getter>
    inline void mutate_and_verify_optional_member(const OrigT& orig, CloneFactory&& make_clone, Getter&& get_opt)
    {
        {
            auto mutated = make_clone();
            auto& opt = get_opt(mutated);
            if (opt.has_value())
            {
                opt.reset();
                assert_mutation_breaks_equality(orig, mutated);
            }
        }
        {
            auto sample = make_clone();
            auto& opt = get_opt(sample);
            if (opt.has_value())
            {
                if constexpr (is_unique_ptr_v<ValT>)
                {
                    using ChildT = typename ValT::element_type;
                    if (opt.get())
                    {
                        if constexpr (HasAstNodeSchema<ChildT>)
                        {
                            mutate_concrete_ast_node_members<ChildT>(orig, make_clone, [&](auto& root) -> ChildT& {
                                return *get_opt(root).get();
                            });
                        }
                        else
                        {
                            NodeDispatcher<AllAstNodeTypes>::dispatch(opt.get()->kind, [&]<typename ConcreteChild>() {
                                if constexpr (std::derived_from<ConcreteChild, ChildT> && HasAstNodeSchema<ConcreteChild>)
                                {
                                    mutate_concrete_ast_node_members<ConcreteChild>(orig, make_clone, [&](auto& root) -> ConcreteChild& {
                                        return *static_cast<ConcreteChild*>(get_opt(root).get());
                                    });
                                }
                            });
                        }
                    }
                }
                else if constexpr (std::same_as<ValT, std::string>)
                {
                    auto mutated = make_clone();
                    *get_opt(mutated) += "_mutated_optional_text";
                    assert_mutation_breaks_equality(orig, mutated);
                }
            }
        }
    }

    template <typename NodeT, size_t Is>
    inline void test_schema_member_mutation(const NodeT& original)
    {
        using MemberRefT = decltype(std::declval<NodeT&>().*std::get<Is>(AstNodeSchema<NodeT>::members));
        using RawMemberT = std::remove_cvref_t<MemberRefT>;

        auto make_clone = [&]() {
            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                return clone_node(original);
            }
            else
            {
                return clone_node(&original);
            }
        };

        auto get_member = [&](auto& root) -> decltype(auto) {
            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                return root.*std::get<Is>(AstNodeSchema<NodeT>::members);
            }
            else
            {
                return (*root).*std::get<Is>(AstNodeSchema<NodeT>::members);
            }
        };

        if constexpr (std::same_as<RawMemberT, SourceSpan> || std::same_as<RawMemberT, valuascript::shared::SourceSpan>)
        {
            mutate_and_verify_span_subproperties(original, make_clone, get_member);
        }
        else if constexpr (std::same_as<RawMemberT, NodeName>)
        {
            mutate_and_verify_nodename(original, make_clone, get_member);
        }
        else if constexpr (std::same_as<RawMemberT, std::string>)
        {
            auto mutated = make_clone();
            get_member(mutated) += "_mutated_string_val";
            assert_mutation_breaks_equality(original, mutated);
        }
        else if constexpr (std::same_as<RawMemberT, bool>)
        {
            auto mutated = make_clone();
            auto& b = get_member(mutated);
            b = !b;
            assert_mutation_breaks_equality(original, mutated);
        }
        else if constexpr (std::same_as<RawMemberT, TokenType>)
        {
            auto mutated = make_clone();
            auto& tok = get_member(mutated);
            tok = (tok == TokenType::Plus ? TokenType::Minus : TokenType::Plus);
            assert_mutation_breaks_equality(original, mutated);
        }
        else if constexpr (std::same_as<RawMemberT, AstKind>)
        {
            // Kind mismatch is tested in ast_equality_kind_mismatch_test
        }
        else if constexpr (IsUniquePtrOfAstNode<RawMemberT>)
        {
            using ChildT = typename RawMemberT::element_type;
            mutate_and_verify_child_pointer<ChildT>(original, make_clone, get_member);
        }
        else if constexpr (IsInnerAstNode<RawMemberT>)
        {
            mutate_concrete_ast_node_members<RawMemberT>(original, make_clone, get_member);
        }
        else if constexpr (is_optional_ast_field_v<RawMemberT>)
        {
            using ValT = typename RawMemberT::value_type;
            mutate_and_verify_optional_member<ValT>(original, make_clone, get_member);
        }
        else if constexpr (is_std_optional_v<RawMemberT>)
        {
            using ValT = typename RawMemberT::value_type;
            mutate_and_verify_optional_member<ValT>(original, make_clone, get_member);
        }
        else if constexpr (requires { typename RawMemberT::value_type; })
        {
            using ElemT = typename RawMemberT::value_type;
            mutate_and_verify_vector_member<ElemT>(original, make_clone, get_member);
        }
    }

    template <typename NodeT>
    inline void run_schema_driven_mutation_test()
    {
        AstFactoryConfig config;
        config.function_parameter.default_value.value = true;
        config.enum_case.value.value = true;
        config.switch_case.identifiers.count = 2;
        config.type_annotation.generic_args.count = 2;
        config.tuple_type_annotation.generic_args.count = 2;
        config.tuple_type_annotation.element_types.count = 2;

        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            NodeT orig = create_sample<NodeT>(2, config);
            EXPECT_TRUE(ast_equals(orig, orig));

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                (test_schema_member_mutation<NodeT, Is>(orig), ...);
            }(std::make_index_sequence<N>{});
        }
        else
        {
            auto orig = create_sample<NodeT>(2, config);
            ASSERT_NE(orig, nullptr);
            EXPECT_TRUE(ast_equals(orig, orig));

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                (test_schema_member_mutation<NodeT, Is>(*orig), ...);
            }(std::make_index_sequence<N>{});
        }
    }

    struct AstEqualityMutationTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstEqualityMutationTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    AstEqualityMutationTestDescriptor make_equality_mutation_test_descriptor()
    {
        return AstEqualityMutationTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []() {
                run_schema_driven_mutation_test<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstEqualityMutationTestDescriptorCollector;

    template <typename... Types>
    struct AstEqualityMutationTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstEqualityMutationTestDescriptor> collect()
        {
            return { make_equality_mutation_test_descriptor<Types>()... };
        }
    };

    class AstEqualityMutationParameterizedTest : public testing::TestWithParam<AstEqualityMutationTestDescriptor>
    {
    };

    TEST_P(AstEqualityMutationParameterizedTest, MutatingAnySchemaMemberBreaksEquality)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Schema Member Mutation Equality Rejection for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstEqualityMutationTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstEqualityMutationTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstEqualityMutationParameterizedTest,
        testing::ValuesIn((AstEqualityMutationTestDescriptorCollector<AllAstNodeTypes>::collect())),
        AstEqualityMutationTestNameGenerator{}
    );
}
