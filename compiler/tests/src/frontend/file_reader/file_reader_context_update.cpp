#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "frontend/file_reader/file_reader_stage.h"
#include "core/compiler_context.h"
#include "utils/pid.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class FileReaderStageTest : public ::testing::Test {
    protected:
        std::filesystem::path temp_dir;
        std::string temp_file_path;
        std::string expected_source = "let x = 100;\nfunc test() { return x; }";

        void SetUp() override {
            temp_dir = generate_test_workspace("vs_registry_test", reinterpret_cast<uintptr_t>(this));

            temp_file_path = (temp_dir / "test_registry_source.vs").string();

            std::ofstream out(temp_file_path, std::ios::binary);
            out << expected_source;
            out.close();
        }

        void TearDown() override {
            cleanup_test_workspace(temp_dir);
        }
    };

    TEST_F(FileReaderStageTest, PopulatesSourceRegistryInContext) {
        auto context = std::make_shared<CompilerContext>();
        FileReaderStage reader;

        std::vector<CompilerStageArtifact> initial_artifacts = {
            {CompilerStageArtifactCode::FilePath, temp_file_path}
        };

        auto result_artifact = reader.run(*context, initial_artifacts);

        std::string expected_key = std::filesystem::weakly_canonical(temp_file_path).string();

        EXPECT_TRUE(context->source_registry.contains(expected_key))
                << "Registry check failed.\n"
                << "Expected Key: " << expected_key << "\n"
                << "Registry has " << context->source_registry.size() << " elements.";

        EXPECT_EQ(context->source_registry[expected_key], expected_source);
    }
}
