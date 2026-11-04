#include <gtest/gtest.h>
#include "type_checker_test_base.h"

namespace valuascript::compiler::test
{
    class TypeCheckerRecoveryTest : public TypeCheckerTestBase
    {
    };

    TEST_F(TypeCheckerRecoveryTest, DetectsTypeMismatchInExplicitVariableDeclaration)
    {
        std::string main_path = CreateFile("main.vs",
            "let x: int = \"not an int\"\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::TypeMismatch));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsTypeMismatchInReassignment)
    {
        std::string main_path = CreateFile("main.vs",
            "let x = 10\n"
            "x = \"string\"\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::TypeMismatch));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsCannotApplyBinaryOperator)
    {
        std::string main_path = CreateFile("main.vs",
            "let x = \"hello\" - 42\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::CannotApplyBinaryOperator));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsCannotApplyUnaryOperator)
    {
        std::string main_path = CreateFile("main.vs",
            "let x = not \"hello\"\n"
        );
        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::CannotApplyUnaryOperator));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsConditionNotBoolean)
    {
        std::string main_path = CreateFile("main.vs",
            "let x = if 123 then 1 else 2\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::ConditionNotBoolean));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsArgumentCountMismatch)
    {
        std::string main_path = CreateFile("main.vs",
            "func add(x: int, y: int) -> int {\n"
            "    return x + y\n"
            "}\n"
            "let res = add(x: 10)\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::ArgumentCountMismatch));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsArgumentLabelMismatch)
    {
        std::string main_path = CreateFile("main.vs",
            "func add(x: int, y: int) -> int {\n"
            "    return x + y\n"
            "}\n"
            "let res = add(a: 10, y: 20)\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::ArgumentLabelMismatch));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsArgumentTypeMismatch)
    {
        std::string main_path = CreateFile("main.vs",
            "func add(x: int, y: int) -> int {\n"
            "    return x + y\n"
            "}\n"
            "let res = add(x: \"string\", y: 20)\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::TypeMismatch));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsInvalidReturnType)
    {
        std::string main_path = CreateFile("main.vs",
            "func greeting() -> string {\n"
            "    return 42\n"
            "}\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::InvalidReturnType));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsNoSuchFieldOnStruct)
    {
        std::string main_path = CreateFile("main.vs",
            "struct Point {\n"
            "    x: float,\n"
            "    y: float\n"
            "}\n"
            "let p = Point(x: 1.0, y: 2.0)\n"
            "let z = p.z\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::NoSuchField));
    }

    TEST_F(TypeCheckerRecoveryTest, DetectsNotCallableOnNonFunction)
    {
        std::string main_path = CreateFile("main.vs",
            "let num = 42\n"
            "let res = num(x: 1)\n"
        );

        auto context = std::make_shared<CompilerContext>();
        RunPipeline(main_path, /*fail_fast=*/false, context);

        const auto& errors = context->diagnostics.get_errors();
        ASSERT_GE(errors.size(), 1);
        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Semantic);
        EXPECT_TRUE(errors[0].is_error(TypeCheckerErrorCode::NotCallable));
    }
}
