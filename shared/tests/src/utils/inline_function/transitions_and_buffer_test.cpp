#include <gtest/gtest.h>
#include "utils/inline_function/helpers.h"

using namespace valuascript::shared;
using namespace valuascript::shared::test;

TEST(InlineFunctionTransitionTest, ReassignSmallCallableToLargeCallable)
{
    InlineFunction<int(), 32> f = []() { return 10; };
    EXPECT_EQ(f(), 10);

    SizedCallable<128> large{.payload = {}, .val = 20};
    f = large;
    EXPECT_EQ(f(), 20);
}

TEST(InlineFunctionTransitionTest, ReassignLargeCallableToSmallCallable)
{
    SizedCallable<128> large{.payload = {}, .val = 20};
    InlineFunction<int(), 32> f = large;
    EXPECT_EQ(f(), 20);

    f = []() { return 30; };
    EXPECT_EQ(f(), 30);
}

TEST(InlineFunctionTransitionTest, ReassignLargeCallableToNullptr)
{
    SizedCallable<128> large{.payload = {}, .val = 20};
    InlineFunction<int(), 32> f = large;
    EXPECT_TRUE(static_cast<bool>(f));

    f = nullptr;
    EXPECT_FALSE(static_cast<bool>(f));
}

TEST(InlineFunctionTransitionTest, CustomBufferSize16)
{
    InlineFunction<int(), 16> f = [a = 1, b = 2]() { return a + b; };
    EXPECT_EQ(f(), 3);
}

TEST(InlineFunctionTransitionTest, CustomBufferSize128)
{
    SizedCallable<100> callable{.payload = {}, .val = 500};
    InlineFunction<int(), 128> f = callable;
    EXPECT_EQ(f(), 500);
}

TEST(InlineFunctionTransitionTest, CustomBufferSize256)
{
    SizedCallable<200> callable{.payload = {}, .val = 900};
    InlineFunction<int(), 256> f = callable;
    EXPECT_EQ(f(), 900);
}
