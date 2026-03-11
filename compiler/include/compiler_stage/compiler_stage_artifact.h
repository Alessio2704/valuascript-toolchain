#pragma once

#include <vector>
#include <any>
#include <stdexcept>
#include <algorithm>

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
            throw std::runtime_error("Extraction Failed: Artifact code not found in pipeline history.");
        }

        try {
            return std::any_cast<ExpectedType>(it->data);
        } catch (const std::bad_any_cast &) {
            throw std::runtime_error("Extraction Failed: Artifact type mismatch during cast.");
        }
    }
}