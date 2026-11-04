#include <gtest/gtest.h>
#include "midend/symbol_resolver/symbol.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    TEST(SymbolTest, ConstructsSymbolPropertiesCorrectly)
    {
        SourceSpan span{.line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5, .start_offset = 0, .length = 4};
        Symbol sym("my_var", SymbolKind::Variable, span);

        EXPECT_EQ(sym.get_name(), "my_var");
        EXPECT_EQ(sym.get_kind(), SymbolKind::Variable);
        EXPECT_EQ(sym.get_span(), span);
        EXPECT_EQ(sym.get_ast_node(), nullptr);
        EXPECT_EQ(sym.get_declaring_scope(), nullptr);

        EXPECT_TRUE(sym.is_value());
        EXPECT_FALSE(sym.is_type());
        EXPECT_FALSE(sym.is_callable());
    }

    TEST(SymbolTest, CorrectlyIdentifiesTypeSymbols)
    {
        SourceSpan span;
        Symbol struct_sym("Point", SymbolKind::Struct, span);
        Symbol enum_sym("Color", SymbolKind::Enum, span);
        Symbol alias_sym("IntAlias", SymbolKind::TypeAlias, span);

        EXPECT_TRUE(struct_sym.is_type());
        EXPECT_FALSE(struct_sym.is_value());
        EXPECT_FALSE(struct_sym.is_callable());

        EXPECT_TRUE(enum_sym.is_type());
        EXPECT_FALSE(enum_sym.is_value());

        EXPECT_TRUE(alias_sym.is_type());
        EXPECT_FALSE(alias_sym.is_value());
    }

    TEST(SymbolTest, CorrectlyIdentifiesCallableSymbols)
    {
        SourceSpan span;
        Symbol fn_sym("compute", SymbolKind::Function, span);

        EXPECT_TRUE(fn_sym.is_callable());
        EXPECT_FALSE(fn_sym.is_type());
        EXPECT_FALSE(fn_sym.is_value());
    }

    TEST(SymbolTest, CorrectlyIdentifiesValueSymbols)
    {
        SourceSpan span;
        Symbol var_sym("x", SymbolKind::Variable, span);
        Symbol param_sym("arg", SymbolKind::Parameter, span);
        Symbol field_sym("x", SymbolKind::StructField, span);
        Symbol case_sym("Red", SymbolKind::EnumCase, span);

        EXPECT_TRUE(var_sym.is_value());
        EXPECT_TRUE(param_sym.is_value());
        EXPECT_TRUE(field_sym.is_value());
        EXPECT_TRUE(case_sym.is_value());

        EXPECT_FALSE(var_sym.is_type());
        EXPECT_FALSE(var_sym.is_callable());
    }

    TEST(SymbolTest, SymbolKindToStringOutputsReadableStrings)
    {
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::Variable), "variable");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::Parameter), "parameter");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::Function), "function");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::Struct), "struct");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::StructField), "field");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::Enum), "enum");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::EnumCase), "enum case");
        EXPECT_EQ(symbol_kind_to_string(SymbolKind::TypeAlias), "type alias");
    }
}
