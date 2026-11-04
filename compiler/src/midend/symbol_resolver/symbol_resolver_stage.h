#pragma once

#include <memory>
#include <string>
#include <vector>
#include "core/compiler_stage.h"
#include "midend/project_resolver/resolved_project_artifact.h"
#include "symbol_table_artifact.h"

namespace valuascript::compiler
{
    class SymbolResolverStage : public CompilerStage
    {
    private:
        void collect_declarations(CompilerContext& context,
                                  const std::string& module_path,
                                  const Program& program,
                                  ScopePtr module_scope,
                                  SymbolTableArtifact& artifact);

        void resolve_bodies(CompilerContext& context,
                            const std::string& module_path,
                            const Program& program,
                            ScopePtr module_scope,
                            SymbolTableArtifact& artifact);

    public:
        SymbolResolverStage();

        CompilerStageArtifact run(CompilerContext& context,
                                  const std::vector<CompilerStageArtifact>& artifacts) override;
    };
}
