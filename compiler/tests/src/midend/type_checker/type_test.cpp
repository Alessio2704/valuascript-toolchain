#include <gtest/gtest.h>
#include "midend/type_checker/type.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    TEST(TypeTest, PrimitiveTypesProperties)
    {
        auto int_t = make_int_type();
        auto float_t = make_float_type();
        auto bool_t = make_bool_type();
        auto string_t = make_string_type();
        auto void_t = make_void_type();
        auto unknown_t = make_unknown_type();

        EXPECT_TRUE(int_t->is_numeric());
        EXPECT_TRUE(float_t->is_numeric());
        EXPECT_FALSE(bool_t->is_numeric());
        EXPECT_FALSE(string_t->is_numeric());

        EXPECT_TRUE(bool_t->is_boolean());
        EXPECT_FALSE(int_t->is_boolean());

        EXPECT_TRUE(string_t->is_string());
        EXPECT_FALSE(int_t->is_string());

        EXPECT_TRUE(void_t->is_void());
        EXPECT_FALSE(int_t->is_void());

        EXPECT_TRUE(unknown_t->is_unknown());
        EXPECT_FALSE(int_t->is_unknown());
    }

    TEST(TypeTest, NumericPromotionAssignability)
    {
        auto int_t = make_int_type();
        auto float_t = make_float_type();

        EXPECT_TRUE(int_t->is_assignable_to(*int_t));
        EXPECT_TRUE(float_t->is_assignable_to(*float_t));
        EXPECT_TRUE(int_t->is_assignable_to(*float_t));  // int promotes to float
        EXPECT_FALSE(float_t->is_assignable_to(*int_t)); // float cannot demote to int
    }

    TEST(TypeTest, UnknownTypeIsUniversallyAssignable)
    {
        auto unknown_t = make_unknown_type();
        auto int_t = make_int_type();
        auto string_t = make_string_type();

        EXPECT_TRUE(unknown_t->is_assignable_to(*int_t));
        EXPECT_TRUE(int_t->is_assignable_to(*unknown_t));
        EXPECT_TRUE(unknown_t->is_assignable_to(*string_t));
        EXPECT_TRUE(string_t->is_assignable_to(*unknown_t));
    }

    TEST(TypeTest, TupleTypeAssignability)
    {
        auto t1 = make_tuple_type({make_int_type(), make_string_type()});
        auto t2 = make_tuple_type({make_int_type(), make_string_type()});
        auto t3 = make_tuple_type({make_int_type(), make_bool_type()});
        auto t4 = make_tuple_type({make_int_type()});

        EXPECT_TRUE(t1->is_assignable_to(*t2));
        EXPECT_FALSE(t1->is_assignable_to(*t3));
        EXPECT_FALSE(t1->is_assignable_to(*t4));
    }

    TEST(TypeTest, StructTypeAssignability)
    {
        auto s1 = make_struct_type("Point");
        auto s2 = make_struct_type("Point");
        auto s3 = make_struct_type("Vector");

        EXPECT_TRUE(s1->is_assignable_to(*s2));
        EXPECT_FALSE(s1->is_assignable_to(*s3));
    }

    TEST(TypeTest, FunctionTypeAssignability)
    {
        auto fn1 = make_function_type({{"x", make_int_type()}, {"y", make_int_type()}}, make_int_type());
        auto fn2 = make_function_type({{"x", make_int_type()}, {"y", make_int_type()}}, make_int_type());
        auto fn_mismatched_label = make_function_type({{"a", make_int_type()}, {"b", make_int_type()}}, make_int_type());
        auto fn_mismatched_ret = make_function_type({{"x", make_int_type()}, {"y", make_int_type()}}, make_string_type());

        EXPECT_TRUE(fn1->is_assignable_to(*fn2));
        EXPECT_FALSE(fn1->is_assignable_to(*fn_mismatched_label));
        EXPECT_FALSE(fn1->is_assignable_to(*fn_mismatched_ret));
    }

    TEST(TypeTest, ToStringOutputsHumanReadableFormatting)
    {
        EXPECT_EQ(make_int_type()->to_string(), "int");
        EXPECT_EQ(make_float_type()->to_string(), "float");
        EXPECT_EQ(make_bool_type()->to_string(), "bool");
        EXPECT_EQ(make_string_type()->to_string(), "string");
        EXPECT_EQ(make_void_type()->to_string(), "void");

        auto tuple_t = make_tuple_type({make_int_type(), make_string_type()});
        EXPECT_EQ(tuple_t->to_string(), "(int, string)");

        auto fn_t = make_function_type({{"x", make_int_type()}, {"y", make_int_type()}}, make_int_type());
        EXPECT_EQ(fn_t->to_string(), "func(x: int, y: int) -> int");
    }
}
