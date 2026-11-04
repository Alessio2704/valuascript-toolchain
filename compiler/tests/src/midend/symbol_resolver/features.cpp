#include <gtest/gtest.h>
#include "symbol_resolver_test_base.h"

namespace valuascript::compiler::test
{
    class SymbolResolverFeaturesTest : public SymbolResolverTestBase
    {
    };

    TEST_F(SymbolResolverFeaturesTest, ResolvesVariablesAndFunctionsInSingleModule)
    {
        std::string main_path = CreateFile("main.vs",
            "let a = 10\n"
            "let b = 20\n"
            "func add(x: int, y: int) -> int {\n"
            "    return x + y\n"
            "}\n"
            "let total = add(x: a, y: b)\n"
        );

        SymbolTableArtifact table;
        ASSERT_NO_THROW({
            table = RunPipeline(main_path);
        });

        auto module_scope = table.get_module_scope(main_path);
        ASSERT_NE(module_scope, nullptr);

        auto sym_a = module_scope->lookup_local("a");
        ASSERT_NE(sym_a, nullptr);
        EXPECT_EQ(sym_a->get_kind(), SymbolKind::Variable);

        auto sym_b = module_scope->lookup_local("b");
        ASSERT_NE(sym_b, nullptr);
        EXPECT_EQ(sym_b->get_kind(), SymbolKind::Variable);

        auto sym_add = module_scope->lookup_local("add");
        ASSERT_NE(sym_add, nullptr);
        EXPECT_EQ(sym_add->get_kind(), SymbolKind::Function);

        auto sym_total = module_scope->lookup_local("total");
        ASSERT_NE(sym_total, nullptr);
        EXPECT_EQ(sym_total->get_kind(), SymbolKind::Variable);
    }

    TEST_F(SymbolResolverFeaturesTest, ResolvesStructAndFields)
    {
        std::string main_path = CreateFile("main.vs",
            "struct Point {\n"
            "    x: float,\n"
            "    y: float\n"
            "}\n"
        );

        SymbolTableArtifact table;
        ASSERT_NO_THROW({
            table = RunPipeline(main_path);
        });

        auto module_scope = table.get_module_scope(main_path);
        ASSERT_NE(module_scope, nullptr);

        auto sym_point = module_scope->lookup_local("Point");
        ASSERT_NE(sym_point, nullptr);
        EXPECT_EQ(sym_point->get_kind(), SymbolKind::Struct);

        auto struct_scope = table.get_scope_at(sym_point->get_ast_node());
        ASSERT_NE(struct_scope, nullptr);
        EXPECT_EQ(struct_scope->get_kind(), ScopeKind::Struct);

        auto sym_x = struct_scope->lookup_local("x");
        ASSERT_NE(sym_x, nullptr);
        EXPECT_EQ(sym_x->get_kind(), SymbolKind::StructField);

        auto sym_y = struct_scope->lookup_local("y");
        ASSERT_NE(sym_y, nullptr);
        EXPECT_EQ(sym_y->get_kind(), SymbolKind::StructField);
    }

    TEST_F(SymbolResolverFeaturesTest, ResolvesEnumAndCases)
    {
        std::string main_path = CreateFile("main.vs",
            "enum Color: int {\n"
            "    Red,\n"
            "    Green,\n"
            "    Blue\n"
            "}\n"
        );

        SymbolTableArtifact table;
        ASSERT_NO_THROW({
            table = RunPipeline(main_path);
        });

        auto module_scope = table.get_module_scope(main_path);
        ASSERT_NE(module_scope, nullptr);

        auto sym_color = module_scope->lookup_local("Color");
        ASSERT_NE(sym_color, nullptr);
        EXPECT_EQ(sym_color->get_kind(), SymbolKind::Enum);

        auto enum_scope = table.get_scope_at(sym_color->get_ast_node());
        ASSERT_NE(enum_scope, nullptr);
        EXPECT_EQ(enum_scope->get_kind(), ScopeKind::Enum);

        auto sym_red = enum_scope->lookup_local("Red");
        ASSERT_NE(sym_red, nullptr);
        EXPECT_EQ(sym_red->get_kind(), SymbolKind::EnumCase);

        auto sym_green = enum_scope->lookup_local("Green");
        ASSERT_NE(sym_green, nullptr);
        EXPECT_EQ(sym_green->get_kind(), SymbolKind::EnumCase);
    }

    TEST_F(SymbolResolverFeaturesTest, ResolvesImportedSymbolsAcrossModules)
    {
        std::string math_path = CreateFile("math.vs", "let pi = 3.14");
        std::string main_path = CreateFile("main.vs",
            "import \"math.vs\"\n"
            "let radius = 2.0\n"
            "let area = pi * radius * radius\n"
        );

        SymbolTableArtifact table;
        ASSERT_NO_THROW({
            table = RunPipeline(main_path);
        });

        auto math_scope = table.get_module_scope(math_path);
        ASSERT_NE(math_scope, nullptr);
        EXPECT_NE(math_scope->lookup_local("pi"), nullptr);

        auto main_scope = table.get_module_scope(main_path);
        ASSERT_NE(main_scope, nullptr);

        auto imported_pi = main_scope->lookup_local("pi");
        ASSERT_NE(imported_pi, nullptr);
        EXPECT_EQ(imported_pi->get_name(), "pi");

        EXPECT_NE(main_scope->lookup_local("radius"), nullptr);
        EXPECT_NE(main_scope->lookup_local("area"), nullptr);
    }

    TEST_F(SymbolResolverFeaturesTest, ResolvesTypeAliases)
    {
        std::string main_path = CreateFile("main.vs",
            "typealias Real = float\n"
            "let x: Real = 1.0\n"
        );

        SymbolTableArtifact table;
        ASSERT_NO_THROW({
            table = RunPipeline(main_path);
        });

        auto module_scope = table.get_module_scope(main_path);
        ASSERT_NE(module_scope, nullptr);

        auto sym_real = module_scope->lookup_local("Real");
        ASSERT_NE(sym_real, nullptr);
        EXPECT_EQ(sym_real->get_kind(), SymbolKind::TypeAlias);
    }
}
