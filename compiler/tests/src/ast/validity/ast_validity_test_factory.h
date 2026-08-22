#pragma once

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <ostream>
#include <tuple>
#include <memory>
#include <optional>
#include <utility>

#include "ast/core/ast_validity.h"
#include "ast/sample/ast_sample_factory.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler::test
{
    struct AstValidityTestDescriptor
    {
        std::string type_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstValidityTestDescriptor& desc)
        {
            return os << desc.type_name;
        }
    };

    template <typename T>
    inline void test_single_node_validity()
    {
        const T* null_raw = nullptr;
        EXPECT_FALSE(ast_is_valid(null_raw));

        std::unique_ptr<T> null_uptr = nullptr;
        EXPECT_FALSE(ast_is_valid(null_uptr));

        std::optional<std::unique_ptr<T>> null_opt_uptr = std::nullopt;
        EXPECT_TRUE(ast_is_valid(null_opt_uptr));

        auto sample = create_sample<T>();
        if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
        {
            ASSERT_NE(sample, nullptr);
            EXPECT_TRUE(ast_is_valid(sample));
            EXPECT_TRUE(ast_is_valid(sample.get()));
            EXPECT_TRUE(ast_is_valid(*sample));

            std::vector<std::unique_ptr<T>> empty_vec;
            EXPECT_TRUE(ast_is_valid(empty_vec));

            std::vector<std::unique_ptr<T>> valid_vec;
            valid_vec.push_back(std::move(sample));
            EXPECT_TRUE(ast_is_valid(valid_vec));

            valid_vec.push_back(nullptr);
            EXPECT_FALSE(ast_is_valid(valid_vec));
        }
        else
        {
            EXPECT_TRUE(ast_is_valid(sample));
            EXPECT_TRUE(ast_is_valid(&sample));

            auto sample2 = create_sample<T>();
            auto sample_uptr = std::make_unique<T>(std::move(sample2));
            EXPECT_TRUE(ast_is_valid(sample_uptr));

            auto sample3 = create_sample<T>();
            std::optional<T> opt_val{std::move(sample3)};
            EXPECT_TRUE(ast_is_valid(opt_val));

            std::vector<T> empty_vec;
            EXPECT_TRUE(ast_is_valid(empty_vec));

            std::vector<T> valid_vec;
            valid_vec.push_back(std::move(sample));
            EXPECT_TRUE(ast_is_valid(valid_vec));

            if constexpr (std::is_default_constructible_v<T>)
            {
                T invalid_default{};
                EXPECT_FALSE(ast_is_valid(invalid_default));
                EXPECT_FALSE(ast_is_valid(&invalid_default));

                T invalid_default2{};
                std::optional<T> opt_invalid{std::move(invalid_default2)};
                EXPECT_FALSE(ast_is_valid(opt_invalid));

                valid_vec.push_back(std::move(invalid_default));
                EXPECT_FALSE(ast_is_valid(valid_vec));
            }
        }
    }

    template <typename T>
    AstValidityTestDescriptor make_ast_validity_test_descriptor()
    {
        return AstValidityTestDescriptor{
            .type_name = std::string(to_string(T::KIND)),
            .run_test = []() {
                test_single_node_validity<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstValidityTestDescriptorCollector;

    template <typename... Types>
    struct AstValidityTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstValidityTestDescriptor> collect()
        {
            return { make_ast_validity_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstValidityTestDescriptor> get_all_ast_validity_test_descriptors()
    {
        return AstValidityTestDescriptorCollector<AllAstNodeTypes>::collect();
    }

    template <typename T>
    struct LeafSampleProvider;

    template <>
    struct LeafSampleProvider<std::string>
    {
        static std::string valid() { return "valid_string"; }
        static std::optional<std::string> invalid() { return std::string(""); }
        static std::string type_name() { return "std_string"; }
    };

    template <>
    struct LeafSampleProvider<bool>
    {
        static bool valid() { return true; }
        static std::optional<bool> invalid() { return std::nullopt; }
        static std::string type_name() { return "bool"; }
    };

    template <>
    struct LeafSampleProvider<NodeName>
    {
        static NodeName valid() { return sample_name("valid_node"); }
        static std::optional<NodeName> invalid() { return NodeName("", sample_span()); }
        static std::string type_name() { return "NodeName"; }
    };

    template <typename T>
    inline void test_leaf_validity()
    {
        auto valid_val = LeafSampleProvider<T>::valid();
        EXPECT_TRUE(ast_is_valid(valid_val));

        std::optional<T> null_opt = std::nullopt;
        EXPECT_TRUE(ast_is_valid(null_opt));

        std::optional<T> valid_opt = valid_val;
        EXPECT_TRUE(ast_is_valid(valid_opt));

        std::vector<T> empty_vec;
        EXPECT_TRUE(ast_is_valid(empty_vec));

        std::vector<T> valid_vec = {valid_val, valid_val};
        EXPECT_TRUE(ast_is_valid(valid_vec));

        if (auto invalid_val = LeafSampleProvider<T>::invalid(); invalid_val.has_value())
        {
            EXPECT_FALSE(ast_is_valid(*invalid_val));

            std::optional<T> invalid_opt = *invalid_val;
            EXPECT_FALSE(ast_is_valid(invalid_opt));

            valid_vec.push_back(*invalid_val);
            EXPECT_FALSE(ast_is_valid(valid_vec));
        }
    }

    template <typename T>
    AstValidityTestDescriptor make_leaf_validity_test_descriptor()
    {
        return AstValidityTestDescriptor{
            .type_name = LeafSampleProvider<T>::type_name(),
            .run_test = []() {
                test_leaf_validity<T>();
            }
        };
    }

    using AstLeafTypes = std::tuple<std::string, bool, NodeName>;

    template <typename Tuple>
    struct LeafValidityTestDescriptorCollector;

    template <typename... Types>
    struct LeafValidityTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstValidityTestDescriptor> collect()
        {
            return { make_leaf_validity_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstValidityTestDescriptor> get_all_leaf_validity_test_descriptors()
    {
        return LeafValidityTestDescriptorCollector<AstLeafTypes>::collect();
    }
}
