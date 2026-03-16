#pragma once

#include <vector>
#include <any>
#include <algorithm>
#include "errors/internal_compiler_exception.h"

namespace valuascript::compiler {
    enum class CompilerStageArtifactCode {
        FilePath,
        SourceCode,
        TokenStream,
        Ast,
        ResolvedProject,
        SymbolTable,
        Bytecode,
        LinterReport
    };

    struct CompilerStageArtifact {
        CompilerStageArtifactCode code;
        std::any data;
    };

    template<typename ExpectedType>
    ExpectedType extract_artifact_data(const std::vector<CompilerStageArtifact> &artifacts,
                                       CompilerStageArtifactCode target_code) {
        auto it = std::find_if(artifacts.begin(), artifacts.end(), [target_code](const CompilerStageArtifact &a) {
            return a.code == target_code;
        });

        if (it == artifacts.end()) {
            throw InternalCompilerException(
                InternalErrorCode::MissingArtifactDuringExtraction,
                static_cast<int>(target_code)
            );
        }

        try {
            return std::any_cast<ExpectedType>(it->data);
        } catch (const std::bad_any_cast &) {
            throw InternalCompilerException(
                InternalErrorCode::InvalidArtifactCast,
                static_cast<int>(target_code),
                typeid(ExpectedType).name()
            );
        }
    }

    CompilerStageArtifact extract_artifact(const std::vector<CompilerStageArtifact> &artifacts,
                                           CompilerStageArtifactCode target_code);
}
