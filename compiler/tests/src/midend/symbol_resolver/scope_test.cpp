#include <gtest/gtest.h>
#include "midend/symbol_resolver/scope.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    TEST(ScopeTest, RootModuleScopeProperties)
    {
        auto root = std::make_shared<Scope>(ScopeKind::Module, "main.vs");

        EXPECT_EQ(root->get_kind(), ScopeKind::Module);
        EXPECT_EQ(root->get_name(), "main.vs");
        EXPECT_EQ(root->get_parent(), nullptr);
        EXPECT_TRUE(root->is_root());
        EXPECT_EQ(root->symbol_count(), 0);
        EXPECT_TRUE(root->get_children().empty());
    }

    TEST(ScopeTest, InsertAndLocalLookup)
    {
        auto scope = std::make_shared<Scope>(ScopeKind::Module, "test.vs");
        SourceSpan span{.line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5, .start_offset = 0, .length = 4};
        auto sym = std::make_shared<Symbol>("foo", SymbolKind::Variable, span);

        auto conflict = scope->insert(sym);
        EXPECT_FALSE(conflict.has_value());
        EXPECT_EQ(scope->symbol_count(), 1);
        EXPECT_EQ(sym->get_declaring_scope(), scope.get());

        auto found = scope->lookup_local("foo");
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->get_name(), "foo");

        EXPECT_EQ(scope->lookup_local("nonexistent"), nullptr);
    }

    TEST(ScopeTest, DuplicateInsertReportsConflict)
    {
        auto scope = std::make_shared<Scope>(ScopeKind::Module, "test.vs");
        SourceSpan span1{.line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5, .start_offset = 0, .length = 4};
        SourceSpan span2{.line_start = 2, .column_start = 1, .line_end = 2, .column_end = 5, .start_offset = 10, .length = 4};

        auto sym1 = std::make_shared<Symbol>("foo", SymbolKind::Variable, span1);
        auto sym2 = std::make_shared<Symbol>("foo", SymbolKind::Function, span2);

        EXPECT_FALSE(scope->insert(sym1).has_value());

        auto conflict = scope->insert(sym2);
        ASSERT_TRUE(conflict.has_value());
        EXPECT_EQ((*conflict)->get_span(), span1);
        EXPECT_EQ((*conflict)->get_kind(), SymbolKind::Variable);
        EXPECT_EQ(scope->symbol_count(), 1);
    }

    TEST(ScopeTest, LexicalLookupAscendsHierarchy)
    {
        auto root = std::make_shared<Scope>(ScopeKind::Module, "module.vs");
        auto fn_scope = root->create_child(ScopeKind::Function, "my_func");
        auto block_scope = fn_scope->create_child(ScopeKind::Block, "inner_block");

        SourceSpan span;
        auto global_sym = std::make_shared<Symbol>("g", SymbolKind::Variable, span);
        auto fn_sym = std::make_shared<Symbol>("param", SymbolKind::Parameter, span);
        auto local_sym = std::make_shared<Symbol>("temp", SymbolKind::Variable, span);

        EXPECT_FALSE(root->insert(global_sym).has_value());
        EXPECT_FALSE(fn_scope->insert(fn_sym).has_value());
        EXPECT_FALSE(block_scope->insert(local_sym).has_value());

        // Block scope can see all three
        EXPECT_EQ(block_scope->lookup_lexical("temp"), local_sym);
        EXPECT_EQ(block_scope->lookup_lexical("param"), fn_sym);
        EXPECT_EQ(block_scope->lookup_lexical("g"), global_sym);
        EXPECT_EQ(block_scope->lookup_lexical("undefined"), nullptr);

        // Function scope can see param and global, but not local
        EXPECT_EQ(fn_scope->lookup_lexical("temp"), nullptr);
        EXPECT_EQ(fn_scope->lookup_lexical("param"), fn_sym);
        EXPECT_EQ(fn_scope->lookup_lexical("g"), global_sym);

        // Root scope can only see global
        EXPECT_EQ(root->lookup_lexical("g"), global_sym);
        EXPECT_EQ(root->lookup_lexical("param"), nullptr);
    }

    TEST(ScopeTest, InnerScopeShadowsOuterSymbol)
    {
        auto outer = std::make_shared<Scope>(ScopeKind::Module, "test.vs");
        auto inner = outer->create_child(ScopeKind::Function, "sub");

        SourceSpan span1{.line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5, .start_offset = 0, .length = 4};
        SourceSpan span2{.line_start = 5, .column_start = 1, .line_end = 5, .column_end = 5, .start_offset = 50, .length = 4};

        auto outer_x = std::make_shared<Symbol>("x", SymbolKind::Variable, span1);
        auto inner_x = std::make_shared<Symbol>("x", SymbolKind::Variable, span2);

        EXPECT_FALSE(outer->insert(outer_x).has_value());
        EXPECT_FALSE(inner->insert(inner_x).has_value());

        EXPECT_EQ(inner->lookup_local("x"), inner_x);
        EXPECT_EQ(inner->lookup_lexical("x"), inner_x);
        EXPECT_EQ(outer->lookup_local("x"), outer_x);
        EXPECT_EQ(outer->lookup_lexical("x"), outer_x);
    }

    TEST(ScopeTest, ScopeKindToStringOutputsReadableStrings)
    {
        EXPECT_EQ(scope_kind_to_string(ScopeKind::Module), "module");
        EXPECT_EQ(scope_kind_to_string(ScopeKind::Function), "function");
        EXPECT_EQ(scope_kind_to_string(ScopeKind::Struct), "struct");
        EXPECT_EQ(scope_kind_to_string(ScopeKind::Enum), "enum");
        EXPECT_EQ(scope_kind_to_string(ScopeKind::Block), "block");
    }
}
