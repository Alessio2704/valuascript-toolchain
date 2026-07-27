#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "midend/project_resolver/project_resolver_stage.h"
#include "midend/project_resolver/project_resolver_error_code.h"
#include "midend/project_resolver/resolved_project_artifact.h"
#include "core/compiler_context.h"
#include "core/compiler_stage_artifact.h"
#include "core/valuascript_exception.h"
#include "utils/pid.h"

namespace valuascript::compiler::test
{
    struct ExpectedResolverError
    {
        ProjectResolverErrorCode code;
        std::optional<std::string> file_path = std::nullopt;
        std::optional<size_t> line = std::nullopt;
        std::optional<size_t> column = std::nullopt;
        std::optional<size_t> line_end = std::nullopt;
        std::optional<size_t> column_end = std::nullopt;

        ExpectedResolverError(ProjectResolverErrorCode c) : code(c) {}
        ExpectedResolverError(ProjectResolverErrorCode c, std::string f)
            : code(c), file_path(std::move(f)) {}
        ExpectedResolverError(ProjectResolverErrorCode c, std::string f, size_t l, size_t col)
            : code(c), file_path(std::move(f)), line(l), column(col) {}
        ExpectedResolverError(ProjectResolverErrorCode c, std::string f, size_t l, size_t col, size_t le, size_t cole)
            : code(c), file_path(std::move(f)), line(l), column(col), line_end(le), column_end(cole) {}
    };

    class ProjectResolverTestBase : public testing::Test
    {
    protected:
        std::filesystem::path temp_dir;

        void SetUp() override
        {
            temp_dir = generate_test_workspace("vs_project_resolver_test", reinterpret_cast<uintptr_t>(this));
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

        ResolvedProjectArtifact RunResolver(const std::string& entry_file,
                                            bool fail_fast = true,
                                            std::shared_ptr<CompilerContext> context = nullptr)
        {
            if (!context)
            {
                context = std::make_shared<CompilerContext>();
            }
            context->settings.fail_fast = fail_fast;

            ProjectResolverStage resolver;
            std::vector<CompilerStageArtifact> input_artifacts = {
                {CompilerStageArtifactCode::FilePath, entry_file}
            };

            auto result_artifact = resolver.run(*context, input_artifacts);
            return extract_artifact_data<ResolvedProjectArtifact>(
                {result_artifact}, CompilerStageArtifactCode::ResolvedProject
            );
        }

        void ExpectResolverSuccess(const std::string& entry_file,
                                   const std::vector<std::string>& expected_topological_order)
        {
            ResolvedProjectArtifact project;
            ASSERT_NO_THROW({
                project = RunResolver(entry_file);
            }) << "ProjectResolverStage threw an unexpected exception for entry file: " << entry_file;

            ASSERT_EQ(project.topological_order.size(), expected_topological_order.size())
                << "Module count in topological order mismatch for entry file: " << entry_file;

            for (size_t i = 0; i < expected_topological_order.size(); ++i)
            {
                EXPECT_EQ(project.topological_order[i], expected_topological_order[i])
                    << "Topological order mismatch at index " << i << " for entry file: " << entry_file;
            }
        }

        void ExpectResolverGraph(const std::string& entry_file,
                                 size_t expected_module_count,
                                 const std::string& expected_bottom_module,
                                 const std::string& expected_top_module)
        {
            ResolvedProjectArtifact project;
            ASSERT_NO_THROW({
                project = RunResolver(entry_file);
            }) << "ProjectResolverStage threw an unexpected exception for entry file: " << entry_file;

            EXPECT_EQ(project.modules.size(), expected_module_count);
            ASSERT_EQ(project.topological_order.size(), expected_module_count);

            EXPECT_EQ(project.topological_order.front(), expected_bottom_module);
            EXPECT_EQ(project.topological_order.back(), expected_top_module);
        }

        void ExpectResolverContainsModules(const std::string& entry_file,
                                           size_t expected_module_count,
                                           const std::vector<std::string>& expected_modules,
                                           const std::string& expected_top_module)
        {
            ResolvedProjectArtifact project;
            ASSERT_NO_THROW({
                project = RunResolver(entry_file);
            }) << "ProjectResolverStage threw an unexpected exception for entry file: " << entry_file;

            EXPECT_EQ(project.modules.size(), expected_module_count);
            ASSERT_EQ(project.topological_order.size(), expected_module_count);

            for (const auto& mod : expected_modules)
            {
                EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), mod),
                          project.topological_order.end())
                    << "Module missing from topological order: " << mod;
            }

            EXPECT_EQ(project.topological_order.back(), expected_top_module);
        }

        void ExpectResolverRecovery(const std::string& entry_file,
                                     const std::vector<ExpectedResolverError>& expected_errors)
        {
            auto context = std::make_shared<CompilerContext>();
            ASSERT_NO_THROW({
                RunResolver(entry_file, /*fail_fast=*/false, context);
            }) << "ProjectResolverStage threw an exception in non-fail-fast mode for entry file: " << entry_file;

            const auto& actual_errors = context->diagnostics.get_errors();
            ASSERT_EQ(actual_errors.size(), expected_errors.size())
                << "Diagnostic error count mismatch in non-fail-fast mode for entry file: " << entry_file;

            for (size_t i = 0; i < expected_errors.size(); ++i)
            {
                const auto& actual = actual_errors[i];
                const auto& expected = expected_errors[i];

                EXPECT_EQ(actual.get_category(), ValuascriptErrorCategory::Import)
                    << "Category mismatch at index " << i;
                EXPECT_TRUE(actual.is_error(expected.code))
                    << "Error code mismatch at index " << i
                    << ". Expected: " << static_cast<int>(expected.code)
                    << ", got: " << actual.get_error_number()
                    << " (" << actual.what() << ")";

                if (expected.file_path.has_value())
                {
                    EXPECT_EQ(actual.get_span().path(), expected.file_path.value())
                        << "File path mismatch at error index " << i << " (" << actual.what() << ")";
                }

                if (expected.line.has_value())
                {
                    EXPECT_EQ(actual.get_span().line_start, expected.line.value())
                        << "Line start mismatch at error index " << i << " (" << actual.what() << ")";
                }

                if (expected.column.has_value())
                {
                    EXPECT_EQ(actual.get_span().column_start, expected.column.value())
                        << "Column start mismatch at error index " << i << " (" << actual.what() << ")";
                }

                if (expected.line_end.has_value())
                {
                    EXPECT_EQ(actual.get_span().line_end, expected.line_end.value())
                        << "Line end mismatch at error index " << i << " (" << actual.what() << ")";
                }

                if (expected.column_end.has_value())
                {
                    EXPECT_EQ(actual.get_span().column_end, expected.column_end.value())
                        << "Column end mismatch at error index " << i << " (" << actual.what() << ")";
                }
            }
        }
    };
}
