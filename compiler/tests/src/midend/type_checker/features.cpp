#include <gtest/gtest.h>
#include "type_checker_test_base.h"

namespace valuascript::compiler::test
{
    class TypeCheckerFeaturesTest : public TypeCheckerTestBase
    {
    };

    TEST_F(TypeCheckerFeaturesTest, InfersArithmeticExpressionTypes)
    {
        std::string main_path = CreateFile("main.vs",
            "let a = 10\n"
            "let b = 20\n"
            "let c = a + b\n"
            "let d = 3.14\n"
            "let e = a + d\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_a = scope->lookup_local("a");
        ASSERT_NE(sym_a, nullptr);
        auto t_a = res.types.get_symbol_type(sym_a.get());
        ASSERT_NE(t_a, nullptr);
        EXPECT_EQ(t_a->get_kind(), TypeKind::Int);

        auto sym_c = scope->lookup_local("c");
        ASSERT_NE(sym_c, nullptr);
        auto t_c = res.types.get_symbol_type(sym_c.get());
        ASSERT_NE(t_c, nullptr);
        EXPECT_EQ(t_c->get_kind(), TypeKind::Int);

        auto sym_d = scope->lookup_local("d");
        ASSERT_NE(sym_d, nullptr);
        auto t_d = res.types.get_symbol_type(sym_d.get());
        ASSERT_NE(t_d, nullptr);
        EXPECT_EQ(t_d->get_kind(), TypeKind::Float);

        auto sym_e = scope->lookup_local("e");
        ASSERT_NE(sym_e, nullptr);
        auto t_e = res.types.get_symbol_type(sym_e.get());
        ASSERT_NE(t_e, nullptr);
        EXPECT_EQ(t_e->get_kind(), TypeKind::Float);
    }

    TEST_F(TypeCheckerFeaturesTest, InfersStringConcatenation)
    {
        std::string main_path = CreateFile("main.vs",
            "let s1 = \"hello \"\n"
            "let s2 = \"world\"\n"
            "let msg = s1 + s2\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_msg = scope->lookup_local("msg");
        ASSERT_NE(sym_msg, nullptr);
        auto t_msg = res.types.get_symbol_type(sym_msg.get());
        ASSERT_NE(t_msg, nullptr);
        EXPECT_EQ(t_msg->get_kind(), TypeKind::String);
    }

    TEST_F(TypeCheckerFeaturesTest, InfersComparisonAndBooleanLogic)
    {
        std::string main_path = CreateFile("main.vs",
            "let cmp = 10 > 5\n"
            "let flag = cmp and true\n"
            "let inv = not flag\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_cmp = scope->lookup_local("cmp");
        ASSERT_NE(sym_cmp, nullptr);
        auto t_cmp = res.types.get_symbol_type(sym_cmp.get());
        ASSERT_NE(t_cmp, nullptr);
        EXPECT_EQ(t_cmp->get_kind(), TypeKind::Bool);

        auto sym_flag = scope->lookup_local("flag");
        ASSERT_NE(sym_flag, nullptr);
        auto t_flag = res.types.get_symbol_type(sym_flag.get());
        ASSERT_NE(t_flag, nullptr);
        EXPECT_EQ(t_flag->get_kind(), TypeKind::Bool);

        auto sym_inv = scope->lookup_local("inv");
        ASSERT_NE(sym_inv, nullptr);
        auto t_inv = res.types.get_symbol_type(sym_inv.get());
        ASSERT_NE(t_inv, nullptr);
        EXPECT_EQ(t_inv->get_kind(), TypeKind::Bool);
    }

    TEST_F(TypeCheckerFeaturesTest, InfersTernaryExpression)
    {
        std::string main_path = CreateFile("main.vs",
            "let val = if true then 100 else 200\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_val = scope->lookup_local("val");
        ASSERT_NE(sym_val, nullptr);
        auto t_val = res.types.get_symbol_type(sym_val.get());
        ASSERT_NE(t_val, nullptr);
        EXPECT_EQ(t_val->get_kind(), TypeKind::Int);
    }

    TEST_F(TypeCheckerFeaturesTest, InfersTupleLiterals)
    {
        std::string main_path = CreateFile("main.vs",
            "let t = (1, 2.5, \"test\", false)\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_t = scope->lookup_local("t");
        ASSERT_NE(sym_t, nullptr);
        auto t_val = res.types.get_symbol_type(sym_t.get());
        ASSERT_NE(t_val, nullptr);
        EXPECT_EQ(t_val->get_kind(), TypeKind::Tuple);
        ASSERT_EQ(t_val->get_element_types().size(), 4);
        EXPECT_EQ(t_val->get_element_types()[0]->get_kind(), TypeKind::Int);
        EXPECT_EQ(t_val->get_element_types()[1]->get_kind(), TypeKind::Float);
        EXPECT_EQ(t_val->get_element_types()[2]->get_kind(), TypeKind::String);
        EXPECT_EQ(t_val->get_element_types()[3]->get_kind(), TypeKind::Bool);
    }

    TEST_F(TypeCheckerFeaturesTest, InfersFunctionCallAndReturn)
    {
        std::string main_path = CreateFile("main.vs",
            "func add(x: int, y: int) -> int {\n"
            "    return x + y\n"
            "}\n"
            "let sum = add(x: 10, y: 20)\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_sum = scope->lookup_local("sum");
        ASSERT_NE(sym_sum, nullptr);
        auto t_sum = res.types.get_symbol_type(sym_sum.get());
        ASSERT_NE(t_sum, nullptr);
        EXPECT_EQ(t_sum->get_kind(), TypeKind::Int);
    }

    TEST_F(TypeCheckerFeaturesTest, InfersStructInstantiationAndFieldAccess)
    {
        std::string main_path = CreateFile("main.vs",
            "struct Point {\n"
            "    x: float,\n"
            "    y: float\n"
            "}\n"
            "let p = Point(x: 1.5, y: 2.5)\n"
            "let px = p.x\n"
            "let py = p.y\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_p = scope->lookup_local("p");
        ASSERT_NE(sym_p, nullptr);
        auto t_p = res.types.get_symbol_type(sym_p.get());
        ASSERT_NE(t_p, nullptr);
        EXPECT_EQ(t_p->get_kind(), TypeKind::Struct);
        EXPECT_EQ(t_p->get_name(), "Point");

        auto sym_px = scope->lookup_local("px");
        ASSERT_NE(sym_px, nullptr);
        auto t_px = res.types.get_symbol_type(sym_px.get());
        ASSERT_NE(t_px, nullptr);
        EXPECT_EQ(t_px->get_kind(), TypeKind::Float);
    }

    TEST_F(TypeCheckerFeaturesTest, SupportsMultiModuleTypeChecking)
    {
        CreateFile("math.vs",
            "@export func square(n: int) -> int {\n"
            "    return n * n\n"
            "}\n"
        );
        std::string main_path = CreateFile("main.vs",
            "import \"math.vs\"\n"
            "let ans = square(n: 5)\n"
        );

        PipelineResult res;
        ASSERT_NO_THROW({
            res = RunPipeline(main_path);
        });

        auto scope = res.symbols.get_module_scope(main_path);
        ASSERT_NE(scope, nullptr);

        auto sym_ans = scope->lookup_local("ans");
        ASSERT_NE(sym_ans, nullptr);
        auto t_ans = res.types.get_symbol_type(sym_ans.get());
        ASSERT_NE(t_ans, nullptr);
        EXPECT_EQ(t_ans->get_kind(), TypeKind::Int);
    }
}
