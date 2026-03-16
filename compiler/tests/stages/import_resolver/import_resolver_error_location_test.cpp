#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "../../../include/errors/valuascript_exception.h"
#include "stages/import_resolver/import_resolver_stage.h"
#include "../../../include/compiler_context/compiler_context.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

class ImportResolverMultiErrorTest : public testing::Test {
protected:
    const std::string main_file = "test_main.vs";
    const std::string module_a_file = "test_module_a.vs";
    const std::string missing_file = "test_missing_module.vs";

    void SetUp() override {
        std::ofstream main_out(main_file);
        main_out << "import \"test_module_a.vs\"\n";
        main_out << "import \"test_missing_module.vs\"\n";
        main_out.close();

        std::ofstream mod_a_out(module_a_file);
        mod_a_out << "import \"test_main.vs\"\n";
        mod_a_out.close();
    }

    void TearDown() override {
        std::filesystem::remove(main_file);
        std::filesystem::remove(module_a_file);
    }
};

TEST_F(ImportResolverMultiErrorTest, CollectsCircularAndMissingFileErrors) {
    auto context = std::make_shared<CompilerContext>();
    context->settings.fail_fast = false;

    std::vector<CompilerStageArtifact> artifacts = {
        {CompilerStageArtifactCode::FilePath, main_file}
    };

    ImportResolverStage resolver;

    ASSERT_NO_THROW({
        resolver.run(*context, artifacts);
        }) << "ImportResolverStage threw an exception even though fail_fast was set to false.";

    const auto &errors = context->diagnostics.get_errors();

    ASSERT_EQ(errors.size(), 2)
        << "Expected exactly 2 import errors (Circular + Missing File), but got " << errors.size();


    EXPECT_EQ(errors[0].get_category(), ErrorCategory::Import);
    EXPECT_EQ(errors[0].get_code(), ErrorCode::CircularImportDetected)
        << "Expected first error to be CircularImportDetected, got: " << static_cast<int>(errors[0].get_code());
    EXPECT_TRUE(
        errors[0].what() != nullptr && std::string(errors[0].what()).find("Circular import") != std::string::npos);

    EXPECT_EQ(errors[1].get_category(), ErrorCategory::Import);
    EXPECT_EQ(errors[1].get_code(), ErrorCode::ImportFileNotFound)
        << "Expected second error to be ImportFileNotFound, got: " << static_cast<int>(errors[1].get_code());
    EXPECT_TRUE(
        errors[1].what() != nullptr && std::string(errors[1].what()).find("test_missing_module.vs") != std::string::
        npos);
}
