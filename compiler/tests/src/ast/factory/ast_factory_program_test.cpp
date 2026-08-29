#include <gtest/gtest.h>
#include <memory>
#include <type_traits>
#include <concepts>

#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast_factory_test_reflection.h"

namespace valuascript::compiler::test
{
    TEST(AstFactoryProgramTest, ProgramFactoryCreatesFullValidHierarchy)
    {
        reset_factory_state();
        auto prog = create_sample_program(0);

        ASSERT_NE(prog, nullptr);
        EXPECT_EQ(prog->kind, AstKind::Program);
        EXPECT_TRUE(prog->is_valid());

        for_each_ast_member(*prog, [](const auto& member)
        {
            using MemberType = std::remove_cvref_t<decltype(member)>;
            if constexpr (std::same_as<MemberType, SourceSpan>)
            {
                EXPECT_TRUE(member.is_valid());
            }
            else if constexpr (is_std_vector_v<MemberType>)
            {
                EXPECT_FALSE(member.empty());
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
            else if constexpr (requires { member->is_valid(); })
            {
                ASSERT_NE(member, nullptr);
                EXPECT_TRUE(member->is_valid());
            }
            else if constexpr (requires { member.is_valid(); })
            {
                EXPECT_TRUE(member.is_valid());
            }
        });
    }

    TEST(AstFactoryProgramTest, ZeroConfigCreatesValidEmptyProgram)
    {
        reset_factory_state();
        AstFactoryConfig zero_cfg{};
        apply_zero_to_node_config<Program>(zero_cfg);

        auto prog = create_sample_program(0, zero_cfg);
        ASSERT_NE(prog, nullptr);
        EXPECT_EQ(prog->kind, AstKind::Program);
        EXPECT_TRUE(prog->is_valid());
        verify_vector_members_empty(*prog);
    }
}
