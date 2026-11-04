#include <gtest/gtest.h>
#include "symbol_resolver_test_base.h"

namespace valuascript::compiler::test
{
    class SymbolResolverRecoveryTest : public SymbolResolverTestBase
    {
    };

    TEST_F(SymbolResolverRecoveryTest, DetectsVariableRedeclarationInSameScope)
    {
        std::string main_path = CreateFile("main.vs",
            "let a = 1\n"
            "let a = 2\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);

        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(SymbolResolverErrorCode::RedeclarationInSameScope));
    }

    TEST_F(SymbolResolverRecoveryTest, DetectsFunctionRedeclarationInSameScope)
    {
        std::string main_path = CreateFile("main.vs",
            "func foo() -> void {}\n"
            "func foo() -> void {}\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);

        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(SymbolResolverErrorCode::RedeclarationInSameScope));
    }

    TEST_F(SymbolResolverRecoveryTest, DetectsUndeclaredIdentifierAtTopLevel)
    {
        std::string main_path = CreateFile("main.vs",
            "let x = undefined_var + 1\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);

        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(SymbolResolverErrorCode::UndeclaredIdentifier));
    }

    TEST_F(SymbolResolverRecoveryTest, DetectsUndeclaredIdentifierInsideFunction)
    {
        std::string main_path = CreateFile("main.vs",
            "func compute() -> int {\n"
            "    return not_found\n"
            "}\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);

        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(SymbolResolverErrorCode::UndeclaredIdentifier));
    }
}
