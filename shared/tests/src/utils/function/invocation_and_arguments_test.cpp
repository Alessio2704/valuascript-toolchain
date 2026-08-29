#include <gtest/gtest.h>
#include "helpers.h"

using namespace valuascript::shared;
using namespace valuascript::shared::test;

TEST(InlineFunctionInvocationTest, VoidReturnNoArguments)
{
    bool executed = false;
    InlineFunction<void()> f = [&executed]() { executed = true; };
    f();
    EXPECT_TRUE(executed);
}

TEST(InlineFunctionInvocationTest, PrimitiveReturnSingleArgument)
{
    InlineFunction<int(int)> f = [](int x) { return x * x; };
    EXPECT_EQ(f(6), 36);
}

TEST(InlineFunctionInvocationTest, PrimitiveReturnMultipleArguments)
{
    InlineFunction<int(int, int, int, int)> f = [](int a, int b, int c, int d) {
        return a + b + c + d;
    };
    EXPECT_EQ(f(1, 2, 3, 4), 10);
}

TEST(InlineFunctionInvocationTest, PassByConstReference)
{
    InlineFunction<std::string(const std::string&, const std::string&)> f = &FreeFunctionConcat;
    std::string s1 = "Valua";
    std::string s2 = "Script";
    EXPECT_EQ(f(s1, s2), "ValuaScript");
}

TEST(InlineFunctionInvocationTest, PassByMutableReferenceMutatesCaller)
{
    InlineFunction<void(int&)> increment = [](int& x) { x += 10; };
    int target = 5;
    increment(target);
    EXPECT_EQ(target, 15);
}

TEST(InlineFunctionInvocationTest, PassByRValueReference)
{
    InlineFunction<std::unique_ptr<int>(std::unique_ptr<int>)> f = [](std::unique_ptr<int> p) {
        *p += 100;
        return p;
    };
    auto ptr = std::make_unique<int>(50);
    auto res = f(std::move(ptr));
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(*res, 150);
}

TEST(InlineFunctionInvocationTest, ReturnMoveOnlyType)
{
    InlineFunction<std::unique_ptr<std::string>()> create_ptr = []() {
        return std::make_unique<std::string>("dynamic_payload");
    };
    auto ptr = create_ptr();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "dynamic_payload");
}

TEST(InlineFunctionInvocationTest, ReturnComplexObject)
{
    InlineFunction<std::vector<int>(int)> generate = [](int n) {
        std::vector<int> v;
        for (int i = 0; i < n; ++i) v.push_back(i);
        return v;
    };
    auto vec = generate(4);
    EXPECT_EQ(vec, (std::vector<int>{0, 1, 2, 3}));
}

TEST(InlineFunctionInvocationTest, EmptyInvocationReturnsDefault)
{
    InlineFunction<int()> empty_int;
    EXPECT_EQ(empty_int(), 0);

    InlineFunction<void()> empty_void;
    empty_void();
}

TEST(InlineFunctionInvocationTest, MutableLambdaMaintainsInternalStateAcrossCalls)
{
    InlineFunction<int()> accumulator = [sum = 0]() mutable {
        sum += 10;
        return sum;
    };
    EXPECT_EQ(accumulator(), 10);
    EXPECT_EQ(accumulator(), 20);
    EXPECT_EQ(accumulator(), 30);
}
