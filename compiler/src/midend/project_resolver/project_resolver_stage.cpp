#include "project_resolver_stage.h"

#include <filesystem>
#include <memory>
#include "core/valuascript_exception.h"
#include "project_resolver_error_code.h"
#include "core/error_formatter.h"
#include "frontend/parser/ast.h"

namespace valuascript::compiler
{
    ProjectResolverStage::ProjectResolverStage()
        : CompilerStage(
            "ProjectProjectResolverStage",
            CompilerStageArtifactCode::ResolvedProject,
            {CompilerStageArtifactCode::FilePath}
        )
    {
    }

    std::string ProjectResolverStage::normalize_path(const std::string& base_file, const std::string& import_path)
    {
        std::filesystem::path base_dir = std::filesystem::path(base_file).parent_path();
        return std::filesystem::weakly_canonical(base_dir / import_path).string();
    }

    void ProjectResolverStage::resolve_recursive(CompilerContext& context,
                                                 const std::string& current_file,
                                                 ResolvedProjectArtifact& project)
    {
        if (resolved_.contains(current_file))
        {
            return;
        }

        resolving_.insert(current_file);

        CompilerStageArtifact ast_artifact = frontend_.run_from_file(context, current_file);

        auto ast = extract_artifact_data<std::shared_ptr<Program>>(
            {ast_artifact}, CompilerStageArtifactCode::Ast
        );

        for (const auto& import_stmt : ast->import_statements)
        {
            std::string clean_path = import_stmt->path;

            if (clean_path.size() >= 2 && clean_path.front() == '"' && clean_path.back() == '"')
            {
                clean_path = clean_path.substr(1, clean_path.size() - 2);
            }

            std::string next_file = normalize_path(current_file, clean_path);

            if (resolving_.contains(next_file))
            {
                ValuaScriptException ex(
                    ValuascriptErrorCategory::Import,
                    ProjectResolverErrorCode::CircularImportDetected,
                    import_stmt->span,
                    format_error(ProjectResolverErrorCode::CircularImportDetected, next_file)
                );

                context.handle_error(ex);
                continue;
            }

            if (!std::filesystem::exists(next_file))
            {
                ValuaScriptException ex(
                    ValuascriptErrorCategory::Import,
                    ProjectResolverErrorCode::ImportFileNotFound,
                    import_stmt->span,
                    format_error(ProjectResolverErrorCode::ImportFileNotFound, clean_path)
                );

                context.handle_error(ex);
                continue;
            }

            resolve_recursive(context, next_file, project);
        }

        project.modules[current_file] = ast;
        project.topological_order.push_back(current_file);

        resolving_.erase(current_file);
        resolved_.insert(current_file);
    }

    CompilerStageArtifact ProjectResolverStage::run(CompilerContext& context,
                                                    const std::vector<CompilerStageArtifact>& artifacts)
    {
        auto raw_file_path = extract_artifact_data<std::string>(
            artifacts, CompilerStageArtifactCode::FilePath
        );

        std::string absolute_file_path = std::filesystem::weakly_canonical(raw_file_path).string();

        ResolvedProjectArtifact project;
        project.entry_file_path = absolute_file_path;

        resolving_.clear();
        resolved_.clear();

        resolve_recursive(context, absolute_file_path, project);

        return {CompilerStageArtifactCode::ResolvedProject, project};
    }
}
