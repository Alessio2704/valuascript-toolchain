#include <gtest/gtest.h>
#include "helpers.h"

using namespace valuascript::shared;
using namespace valuascript::shared::test;

TEST(InlineFunctionSmallCallableTest, CopyConstructCreatesIndependentInstance)
{
    int val = 10;
    InlineFunction<int()> f1 = [val]() { return val; };
    InlineFunction<int()> f2 = f1;

    ASSERT_TRUE(static_cast<bool>(f1));
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(f1(), 10);
    EXPECT_EQ(f2(), 10);
}

TEST(InlineFunctionSmallCallableTest, CopyAssignOverwritesTarget)
{
    InlineFunction<int()> f1 = []() { return 100; };
    InlineFunction<int()> f2 = []() { return 200; };

    f2 = f1;
    EXPECT_EQ(f2(), 100);
}

TEST(InlineFunctionSmallCallableTest, CopyAssignSelfAssignmentSafe)
{
    InlineFunction<int()> f = []() { return 42; };
    InlineFunction<int()>& f_ref = f;
    f = f_ref;
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 42);
}

TEST(InlineFunctionSmallCallableTest, MoveConstructTransfersStateAndEmptiesSource)
{
    InlineFunction<int()> f1 = []() { return 55; };
    InlineFunction<int()> f2 = std::move(f1);

    EXPECT_FALSE(static_cast<bool>(f1));
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(f2(), 55);
}

TEST(InlineFunctionSmallCallableTest, MoveAssignTransfersStateAndEmptiesSource)
{
    InlineFunction<int()> f1 = []() { return 77; };
    InlineFunction<int()> f2 = []() { return 88; };

    f2 = std::move(f1);
    EXPECT_FALSE(static_cast<bool>(f1));
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(f2(), 77);
}

TEST(InlineFunctionSmallCallableTest, MoveAssignSelfAssignmentSafe)
{
    InlineFunction<int()> f = []() { return 99; };
    InlineFunction<int()>& f_ref = f;
    f = std::move(f_ref);
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(), 99);
}

TEST(InlineFunctionSmallCallableTest, DestructorInvokedOnScopeExit)
{
    int dtor_count = 0;
    {
        InlineFunction<int(int)> f = LifecycleTracker(nullptr, nullptr, nullptr, &dtor_count, 10);
        EXPECT_EQ(f(5), 15);
    }
    EXPECT_GT(dtor_count, 0);
}

TEST(InlineFunctionSmallCallableTest, DestructorInvokedOnReset)
{
    int dtor_count = 0;
    InlineFunction<int(int)> f = LifecycleTracker(nullptr, nullptr, nullptr, &dtor_count, 10);
    int before_reset = dtor_count;
    f.reset();
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_GT(dtor_count, before_reset);
}

TEST(InlineFunctionSmallCallableTest, DestructorInvokedOnNullptrAssign)
{
    int dtor_count = 0;
    InlineFunction<int(int)> f = LifecycleTracker(nullptr, nullptr, nullptr, &dtor_count, 10);
    int before = dtor_count;
    f = nullptr;
    EXPECT_FALSE(static_cast<bool>(f));
    EXPECT_GT(dtor_count, before);
}

TEST(InlineFunctionSmallCallableTest, DestructorInvokedOnReassignment)
{
    int dtor1 = 0;
    int dtor2 = 0;
    InlineFunction<int(int)> f = LifecycleTracker(nullptr, nullptr, nullptr, &dtor1, 10);
    f = LifecycleTracker(nullptr, nullptr, nullptr, &dtor2, 20);

    EXPECT_GT(dtor1, 0);
    EXPECT_EQ(f(5), 25);
}

TEST(InlineFunctionSmallCallableTest, ExactLifecycleCountsOnCopyAndMove)
{
    int ctor = 0, copy = 0, move = 0, dtor = 0;
    {
        LifecycleTracker tracker(&ctor, &copy, &move, &dtor, 10);
        InlineFunction<int(int)> f1(tracker);
        InlineFunction<int(int)> f2 = f1;
        InlineFunction<int(int)> f3 = std::move(f1);
    }
    EXPECT_EQ(ctor + copy + move, dtor);
}
