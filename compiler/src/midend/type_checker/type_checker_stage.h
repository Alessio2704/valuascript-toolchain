#pragma once

#include <memory>
#include <string>
#include <vector>
#include "core/compiler_stage.h"
#include "midend/project_resolver/resolved_project_artifact.h"
#include "midend/symbol_resolver/symbol_table_artifact.h"
#include "type_table_artifact.h"

namespace valuascript::compiler
{
    class TypeCheckerStage : public CompilerStage
    {
    private:
        void register_declaration_types(CompilerContext& context,
                                        const Program& program,
                                        Scope* module_scope,
                                        const SymbolTableArtifact& symbol_table,
                                        TypeTableArtifact& artifact);

        void type_check_program(CompilerContext& context,
                                const Program& program,
                                Scope* module_scope,
                                const SymbolTableArtifact& symbol_table,
                                TypeTableArtifact& artifact);

    public:
        TypeCheckerStage();

        static TypePtr resolve_type_annotation(const TypeAnnotation* node, Scope* scope);

        CompilerStageArtifact run(CompilerContext& context,
                                  const std::vector<CompilerStageArtifact>& artifacts) override;
    };
}
