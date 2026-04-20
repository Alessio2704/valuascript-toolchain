#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "utils/pid.h"
#include "core/valuascript_exception.h"
#include "midend/project_resolver/project_resolver_stage.h"
#include "core/compiler_context.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class ProjectResolverMultiErrorTest : public testing::Test
    {
    protected:
        std::filesystem::path temp_dir;
        std::string main_file;
        std::string module_a_file;

        void SetUp() override
        {
            temp_dir = generate_test_workspace("vs_multi", reinterpret_cast<uintptr_t>(this));

            main_file = std::filesystem::weakly_canonical(temp_dir / "test_main.vs").string();
            module_a_file = std::filesystem::weakly_canonical(temp_dir / "test_module_a.vs").string();

            std::ofstream main_out(main_file, std::ios::binary);
            main_out << "import \"test_module_a.vs\"\n";
            main_out << "import \"test_missing_module.vs\"\n";
            main_out.close();

            std::ofstream mod_a_out(module_a_file, std::ios::binary);
            mod_a_out << "import \"test_main.vs\"\n";
            mod_a_out.close();
        }

        void TearDown() override
        {
            cleanup_test_workspace(temp_dir);
        }
    };

    TEST_F(ProjectResolverMultiErrorTest, CollectsCircularAndMissingFileErrors)
    {
        auto context = std::make_shared<CompilerContext>();
        context->settings.fail_fast = false;

        std::vector<CompilerStageArtifact> artifacts = {
            {CompilerStageArtifactCode::FilePath, main_file}
        };

        ProjectResolverStage resolver;

        ASSERT_NO_THROW({
            resolver.run(*context, artifacts);
            }) << "ProjectProjectResolverStage threw an exception even though fail_fast was set to false.";

        const auto& errors = context->diagnostics.get_errors();

        ASSERT_EQ(errors.size(), 2)
            << "Expected exactly 2 import errors (Circular + Missing File), but got " << errors.size();


        EXPECT_EQ(errors[0].get_category(), ValuascriptErrorCategory::Import);
        EXPECT_EQ(errors[0].get_code(), ValuascriptErrorCode::CircularImportDetected)
            << "Expected first error to be CircularImportDetected, got: " << static_cast<int>(errors[0].get_code());
        EXPECT_TRUE(
            errors[0].what() != nullptr && std::string(errors[0].what()).find("Circular import") != std::string::npos);

        EXPECT_EQ(errors[0].get_span().line_start, 1);
        EXPECT_EQ(errors[0].get_span().column_start, 1);
        EXPECT_EQ(errors[0].get_span().line_end, 1);
        EXPECT_EQ(errors[0].get_span().column_end, 22);
        EXPECT_EQ(errors[0].get_span().file_path, module_a_file);

        EXPECT_EQ(errors[1].get_category(), ValuascriptErrorCategory::Import);
        EXPECT_EQ(errors[1].get_code(), ValuascriptErrorCode::ImportFileNotFound)
            << "Expected second error to be ImportFileNotFound, got: " << static_cast<int>(errors[1].get_code());
        EXPECT_TRUE(
            errors[1].what() != nullptr && std::string(errors[1].what()).find("test_missing_module.vs") != std::string::
            npos);

        EXPECT_EQ(errors[1].get_span().line_start, 2);
        EXPECT_EQ(errors[1].get_span().column_start, 1);
        EXPECT_EQ(errors[1].get_span().line_end, 2);
        EXPECT_EQ(errors[1].get_span().column_end, 32);
    }
}
