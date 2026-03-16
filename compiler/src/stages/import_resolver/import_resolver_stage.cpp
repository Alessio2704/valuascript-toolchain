#include "stages/import_resolver/import_resolver_stage.h"

#include <filesystem>
#include <memory>
#include "errors/valuascript_exception.h"
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler {
    ImportResolverStage::ImportResolverStage()
        : CompilerStage(
            "ImportResolverStage",
            CompilerStageArtifactCode::ResolvedProject,
            {CompilerStageArtifactCode::FilePath}
        ) {
    }

    std::string ImportResolverStage::normalize_path(const std::string &base_file, const std::string &import_path) {
        std::filesystem::path base_dir = std::filesystem::path(base_file).parent_path();
        return std::filesystem::weakly_canonical(base_dir / import_path).string();
    }

    void ImportResolverStage::resolve_recursive(CompilerContext &context,
                                                const std::string &current_file, ResolvedProjectArtifact &project) {
        if (resolving_.contains(current_file)) {
            ValuaScriptException ex(
                ErrorCategory::Import,
                ErrorCode::CircularImportDetected,
                {0, 0, 0, 0, current_file},
                format_error(ErrorCode::CircularImportDetected, current_file)
            );

            context.handle_error(ex);
            return;
        }

        if (resolved_.contains(current_file)) {
            return;
        }

        resolving_.insert(current_file);

        CompilerStageArtifact ast_artifact = frontend_.run_from_file(context, current_file);

        auto ast = extract_artifact_data<std::shared_ptr<Program> >(
            {ast_artifact}, CompilerStageArtifactCode::Ast
        );

        for (const auto &import_stmt: ast->import_statements) {
            std::string clean_path = import_stmt->path;

            if (clean_path.size() >= 2 && clean_path.front() == '"' && clean_path.back() == '"') {
                clean_path = clean_path.substr(1, clean_path.size() - 2);
            }

            std::string next_file = normalize_path(current_file, clean_path);

            if (!std::filesystem::exists(next_file)) {
                ValuaScriptException ex(
                    ErrorCategory::Import,
                    ErrorCode::ImportFileNotFound,
                    {0, 0, 0, 0, current_file},
                    format_error(ErrorCode::ImportFileNotFound, clean_path)
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

    CompilerStageArtifact ImportResolverStage::run(CompilerContext &context,
                                                   const std::vector<CompilerStageArtifact> &artifacts) {
        auto raw_file_path = extract_artifact_data<std::string>(
            artifacts, CompilerStageArtifactCode::FilePath
        );

        std::string absolute_file_path = std::filesystem::absolute(raw_file_path).string();

        ResolvedProjectArtifact project;
        project.entry_file_path = absolute_file_path;

        resolving_.clear();
        resolved_.clear();

        resolve_recursive(context, absolute_file_path, project);

        return {CompilerStageArtifactCode::ResolvedProject, project};
    }
}
