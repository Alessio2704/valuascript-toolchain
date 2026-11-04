#include <gtest/gtest.h>
#include "helpers.h"

using namespace valuascript::shared;
using namespace valuascript::shared::test;

TEST(InlineFunctionStateTest, ExplicitBoolConversion)
{
    InlineFunction<void()> empty;
    EXPECT_FALSE(static_cast<bool>(empty));

    InlineFunction<void()> active = []() {};
    EXPECT_TRUE(static_cast<bool>(active));
}

TEST(InlineFunctionStateTest, EqualityWithNullptr)
{
    InlineFunction<void()> empty;
    EXPECT_TRUE(empty == nullptr);
    EXPECT_TRUE(nullptr == empty);

    InlineFunction<void()> active = []() {};
    EXPECT_FALSE(active == nullptr);
    EXPECT_FALSE(nullptr == active);
}

TEST(InlineFunctionStateTest, InequalityWithNullptr)
{
    InlineFunction<void()> empty;
    EXPECT_FALSE(empty != nullptr);
    EXPECT_FALSE(nullptr != empty);

    InlineFunction<void()> active = []() {};
    EXPECT_TRUE(active != nullptr);
    EXPECT_TRUE(nullptr != active);
}

TEST(InlineFunctionStateTest, ResetMakesFunctionEmpty)
{
    InlineFunction<int()> f = []() { return 1; };
    ASSERT_TRUE(static_cast<bool>(f));

    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_EQ(f, nullptr);
}
