#include <gtest/gtest.h>
#include "helpers.h"

using namespace valuascript::shared;
using namespace valuascript::shared::test;

TEST(InlineFunctionConstructionTest, DefaultConstructedIsEmpty)
{
    InlineFunction<void()> f;
    EXPECT_FALSE(static_cast<bool>(f));
}

TEST(InlineFunctionConstructionTest, NullptrConstructedIsEmpty)
{
    InlineFunction<int(int)> f(nullptr);
    EXPECT_FALSE(static_cast<bool>(f));
}

TEST(InlineFunctionConstructionTest, ConstructFromFreeFunctionPointer)
{
    InlineFunction<int(int, int)> f(&FreeFunctionAdd);
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(10, 20), 30);
}

TEST(InlineFunctionConstructionTest, ConstructFromStatelessLambda)
{
    InlineFunction<int(int)> f = [](int x) { return x * 3; };
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(5), 15);
}

TEST(InlineFunctionConstructionTest, ConstructFromStatefulLambda)
{
    std::string prefix = "Hello, ";
    InlineFunction<std::string(const std::string&)> f = [prefix](const std::string& name) {
        return prefix + name;
    };
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f("World"), "Hello, World");
}

TEST(InlineFunctionConstructionTest, ConstructFromFunctorObject)
{
    FunctorMultiply multiplier{.factor = 4};
    InlineFunction<int(int)> f = multiplier;
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(7), 28);
}

TEST(InlineFunctionConstructionTest, ConstructFromMutableLambda)
{
    InlineFunction<int()> counter = [count = 0]() mutable {
        return ++count;
    };
    ASSERT_TRUE(static_cast<bool>(counter));
    EXPECT_EQ(counter(), 1);
    EXPECT_EQ(counter(), 2);
    EXPECT_EQ(counter(), 3);
}

TEST(InlineFunctionConstructionTest, ConstructFromStdFunction)
{
    std::function<int(int, int)> std_fn = [](int a, int b) { return a - b; };
    InlineFunction<int(int, int)> f = std_fn;
    ASSERT_TRUE(static_cast<bool>(f));
    EXPECT_EQ(f(10, 3), 7);
}
