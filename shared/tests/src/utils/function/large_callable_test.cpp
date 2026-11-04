#include <gtest/gtest.h>
#include "helpers.h"

using namespace valuascript::shared;
using namespace valuascript::shared::test;

TEST(InlineFunctionLargeCallableTest, InvocationProducesCorrectResult)
{
    SizedCallable<128> large{.payload = {}, .val = 999};
    InlineFunction<int(), 32> f = large;
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 999);
}

TEST(InlineFunctionLargeCallableTest, CopyConstructSharesState)
{
    SizedCallable<128> large{.payload = {}, .val = 333};
    InlineFunction<int(), 32> f1 = large;
    InlineFunction<int(), 32> f2 = f1;

    ASSERT_TRUE(static_cast<bool>(f1));
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(f1(), 333);
    EXPECT_EQ(f2(), 333);
}

TEST(InlineFunctionLargeCallableTest, CopyAssignOverwritesTarget)
{
    SizedCallable<128> large1{.payload = {}, .val = 111};
    SizedCallable<128> large2{.payload = {}, .val = 222};

    InlineFunction<int(), 32> f1 = large1;
    InlineFunction<int(), 32> f2 = large2;

    f2 = f1;
    EXPECT_EQ(f2(), 111);
}

TEST(InlineFunctionLargeCallableTest, CopyAssignSelfAssignmentSafe)
{
    SizedCallable<128> large{.payload = {}, .val = 777};
    InlineFunction<int(), 32> f = large;
    InlineFunction<int(), 32>& f_ref = f;
    f = f_ref;
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 777);
}

TEST(InlineFunctionLargeCallableTest, MoveConstructTransfersStateAndEmptiesSource)
{
    SizedCallable<128> large{.payload = {}, .val = 444};
    InlineFunction<int(), 32> f1 = large;
    InlineFunction<int(), 32> f2 = std::move(f1);

    EXPECT_FALSE(static_cast<bool>(f1));
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(f2(), 444);
}

TEST(InlineFunctionLargeCallableTest, MoveAssignTransfersStateAndEmptiesSource)
{
    SizedCallable<128> large1{.payload = {}, .val = 555};
    SizedCallable<128> large2{.payload = {}, .val = 666};

    InlineFunction<int(), 32> f1 = large1;
    InlineFunction<int(), 32> f2 = large2;

    f2 = std::move(f1);
    EXPECT_FALSE(static_cast<bool>(f1));
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(f2(), 555);
}

TEST(InlineFunctionLargeCallableTest, MoveAssignSelfAssignmentSafe)
{
    SizedCallable<128> large{.payload = {}, .val = 888};
    InlineFunction<int(), 32> f = large;
    InlineFunction<int(), 32>& f_ref = f;
    f = std::move(f_ref);
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 888);
}

TEST(InlineFunctionLargeCallableTest, DestructorInvokedWhenLastCopyDestroyed)
{
    int dtor = 0;
    {
        InlineFunction<int(), 32> f1 = TrackedLarge(&dtor);
        {
            InlineFunction<int(), 32> f2 = f1;
            EXPECT_EQ(dtor, 0);
        }
        EXPECT_EQ(dtor, 0);
    }
    EXPECT_EQ(dtor, 1);
}

TEST(InlineFunctionLargeCallableTest, DestructorInvokedOnReset)
{
    int dtor = 0;
    InlineFunction<int(), 32> f = TrackedLarge(&dtor);
    EXPECT_EQ(dtor, 0);
    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_EQ(dtor, 1);
}

TEST(InlineFunctionLargeCallableTest, DestructorInvokedOnNullptrAssign)
{
    int dtor = 0;
    InlineFunction<int(), 32> f = TrackedLarge(&dtor);
    EXPECT_EQ(dtor, 0);
    f = nullptr;
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_EQ(dtor, 1);
}

TEST(InlineFunctionLargeCallableTest, DestructorInvokedOnReassignment)
{
    int dtor1 = 0;
    int dtor2 = 0;
    InlineFunction<int(), 32> f = TrackedLarge(&dtor1);
    EXPECT_EQ(dtor1, 0);

    f = TrackedLarge(&dtor2);
    EXPECT_EQ(dtor1, 1);
    EXPECT_EQ(dtor2, 0);
}
