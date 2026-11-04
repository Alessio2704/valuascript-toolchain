#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "midend/project_resolver/project_resolver_stage.h"
#include "midend/symbol_resolver/symbol_resolver_stage.h"
#include "midend/symbol_resolver/symbol_resolver_error_code.h"
#include "midend/symbol_resolver/symbol_table_artifact.h"
#include "core/compiler_context.h"
#include "core/compiler_stage_artifact.h"
#include "core/valuascript_exception.h"
#include "utils/pid.h"

namespace valuascript::compiler::test
{
    class SymbolResolverTestBase : public testing::Test
    {
    protected:
        std::filesystem::path temp_dir;

        void SetUp() override
        {
            temp_dir = generate_test_workspace("vs_symbol_resolver_test", reinterpret_cast<uintptr_t>(this));
        }

        void TearDown() override
        {
            cleanup_test_workspace(temp_dir);
        }

        std::string CreateFile(const std::string& relative_path, const std::string& content)
        {
            std::filesystem::path full_path = temp_dir / relative_path;
            std::filesystem::create_directories(full_path.parent_path());

            std::ofstream out(full_path, std::ios::binary);
            out << content;
            out.close();

            return std::filesystem::weakly_canonical(full_path).string();
        }

        SymbolTableArtifact RunPipeline(const std::string& entry_file,
                                        bool fail_fast = true,
                                        std::shared_ptr<CompilerContext> context = nullptr)
        {
            if (!context)
            {
                context = std::make_shared<CompilerContext>();
            }
            context->settings.fail_fast = fail_fast;

            // 1. Run Project Resolver
            ProjectResolverStage project_resolver;
            std::vector<CompilerStageArtifact> project_inputs = {
                {.code = CompilerStageArtifactCode::FilePath, .data = entry_file}
            };
            auto project_artifact = project_resolver.run(*context, project_inputs);

            // 2. Run Symbol Resolver
            SymbolResolverStage symbol_resolver;
            std::vector<CompilerStageArtifact> symbol_inputs = {project_artifact};
            auto symbol_artifact = symbol_resolver.run(*context, symbol_inputs);

            return extract_artifact_data<SymbolTableArtifact>(
                {symbol_artifact}, CompilerStageArtifactCode::SymbolTable
            );
        }
    };
}
