#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <ostream>

#include "ast/core/ast_core.h"
#include "ast/equality/ast_equality.h"
#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_registry.h"

namespace valuascript::compiler::test
{
    inline std::vector<std::unique_ptr<AstNode>> create_all_node_samples()
    {
        std::vector<std::unique_ptr<AstNode>> samples;
        [&]<typename... Types>(std::tuple<Types...>*) {
            ([&]() {
                if constexpr (valuascript::shared::tuple_contains_type_v<Types, AllInnerNodeTypes>)
                {
                    auto sample = create_sample<Types>(0);
                    samples.push_back(std::make_unique<Types>(std::move(sample)));
                }
                else
                {
                    auto sample = create_sample<Types>(0);
                    samples.push_back(std::move(sample));
                }
            }(), ...);
        }(static_cast<AllAstNodeTypes*>(nullptr));
        return samples;
    }

    struct AstKindMismatchTestDescriptor
    {
        std::string node_name;
        size_t node_index;

        friend std::ostream& operator<<(std::ostream& os, const AstKindMismatchTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename Tuple>
    struct AstKindMismatchTestDescriptorCollector;

    template <typename... Types>
    struct AstKindMismatchTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstKindMismatchTestDescriptor> collect()
        {
            std::vector<AstKindMismatchTestDescriptor> list;
            size_t idx = 0;
            ([&]() {
                list.push_back(AstKindMismatchTestDescriptor{
                    .node_name = std::string(get_ast_node_name<Types>()),
                    .node_index = idx++
                });
            }(), ...);
            return list;
        }
    };

    class AstEqualityKindMismatchParameterizedTest : public testing::TestWithParam<AstKindMismatchTestDescriptor>
    {
    };

    TEST_P(AstEqualityKindMismatchParameterizedTest, CrossKindComparisonsAlwaysReturnFalse)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Kind Mismatch Equality Rejection for: " + descriptor.node_name);

        auto samples = create_all_node_samples();
        ASSERT_LT(descriptor.node_index, samples.size());
        const auto& target_sample = samples[descriptor.node_index];
        ASSERT_NE(target_sample, nullptr);

        for (size_t i = 0; i < samples.size(); ++i)
        {
            if (i == descriptor.node_index) continue;
            const auto& other_sample = samples[i];
            ASSERT_NE(other_sample, nullptr);

            EXPECT_FALSE(ast_equals(target_sample.get(), other_sample.get()));
            EXPECT_FALSE(ast_equals(other_sample.get(), target_sample.get()));
            EXPECT_FALSE(ast_equals(*target_sample, *other_sample));
            EXPECT_FALSE(ast_equals(*other_sample, *target_sample));
        }
    }

    struct AstKindMismatchTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstKindMismatchTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstEqualityKindMismatchParameterizedTest,
        testing::ValuesIn((AstKindMismatchTestDescriptorCollector<AllAstNodeTypes>::collect())),
        AstKindMismatchTestNameGenerator{}
    );
}
