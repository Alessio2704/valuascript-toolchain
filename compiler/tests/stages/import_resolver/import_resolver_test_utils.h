#pragma once
#include "stages/frontend/parser/ast.h"
#include "stages/import_resolver/import_resolver_stage.h"
#include "stages/import_resolver/resolved_project_artifact.h"

using namespace valuascript;
using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    inline ResolvedProjectArtifact run_resolver(const std::string& entry_file) {

        ImportResolverStage resolver;

        std::vector<CompilerStageArtifact> input_artifacts = {
            {CompilerStageArtifactCode::FilePath, entry_file}
        };

        auto context = std::make_shared<CompilerContext>();

        auto output = resolver.run(context, input_artifacts);
        return extract_artifact_data<ResolvedProjectArtifact>(
            {output}, CompilerStageArtifactCode::ResolvedProject
        );
    }
}