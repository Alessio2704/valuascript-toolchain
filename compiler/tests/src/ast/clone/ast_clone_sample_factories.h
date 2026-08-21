#pragma once

#include "ast/core/ast_clone.h"
#include "ast/core/ast_node_registry.h"
#include "ast_clone_test_factory.h"
#include "ast_dummy_generator.h"

namespace valuascript::compiler::test
{

    template <typename Base, typename T>
    inline void test_polymorphic_base_clone(const T& node_instance)
    {
        if constexpr (std::derived_from<T, Base>)
        {
            auto base_clone = clone_node(static_cast<const Base*>(&node_instance));
            ASSERT_NE(base_clone, nullptr);
            EXPECT_NE(base_clone.get(), &node_instance);
            EXPECT_TRUE(is_a<T>(base_clone.get()));
            assert_deep_equal(node_instance, *ast_cast<T>(base_clone.get()));
        }
    }

    template <typename T, typename CategoryTuple>
    struct CategoryPolymorphicTester;

    template <typename T, typename... Categories>
    struct CategoryPolymorphicTester<T, std::tuple<Categories...>>
    {
        static void test(const T& target_node)
        {
            (test_polymorphic_base_clone<Categories>(target_node), ...);
        }
    };

    template <typename T>
    inline void run_pointer_node_test()
    {
        auto null_clone = clone_node(static_cast<const T*>(nullptr));
        EXPECT_EQ(null_clone, nullptr);

        auto orig = AstCloneSampleFactory<T>::create_sample();
        ASSERT_NE(orig, nullptr);

        auto clone1 = clone_node(orig.get());
        ASSERT_NE(clone1, nullptr);
        EXPECT_NE(clone1.get(), orig.get());
        assert_deep_equal(*orig, *clone1);

        auto clone2 = clone_node(orig);
        ASSERT_NE(clone2, nullptr);
        EXPECT_NE(clone2.get(), orig.get());
        EXPECT_NE(clone2.get(), clone1.get());
        assert_deep_equal(*orig, *clone2);

        test_polymorphic_base_clone<AstNode>(*orig);
        CategoryPolymorphicTester<T, AstCategoryTypes>::test(*orig);

        std::vector<std::unique_ptr<T>> vec;
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        auto cloned_vec = clone_nodes(vec);
        ASSERT_EQ(cloned_vec.size(), 2);
        assert_vector_ptr_disjoint_and_deep_equal(vec, cloned_vec);

        auto original_span = orig->span;
        clone1->span.start_offset += 1000;
        clone1->span.line_start += 50;
        EXPECT_EQ(orig->span, original_span);
    }

    template <typename T>
    inline void run_value_node_test()
    {
        T orig = AstCloneSampleFactory<T>::create_sample();
        T cloned = clone_node(orig);
        assert_deep_equal(orig, cloned);

        test_polymorphic_base_clone<AstNode>(orig);
        CategoryPolymorphicTester<T, AstCategoryTypes>::test(orig);

        std::vector<T> vec;
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        auto cloned_vec = clone_nodes(vec);
        ASSERT_EQ(cloned_vec.size(), 2);
        assert_vector_deep_equal(vec, cloned_vec);

        auto original_span = orig.span;
        cloned.span.start_offset += 1000;
        cloned.span.line_start += 50;
        EXPECT_EQ(orig.span, original_span);
    }

    template <typename T>
    struct AstCloneSampleFactory
    {
        static auto create_sample()
        {
            return create_dummy<T>();
        }

        static void run_full_clone_test()
        {
            if constexpr (requires { { create_dummy<T>() } -> std::same_as<std::unique_ptr<T>>; })
            {
                run_pointer_node_test<T>();
            }
            else
            {
                run_value_node_test<T>();
            }
        }
    };
}
