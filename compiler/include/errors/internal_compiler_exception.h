#pragma once
#include <stdexcept>
#include <string>
#include "error_formatter.h"

namespace valuascript::compiler {
    enum class InternalErrorCode {
        MissingDependencyInCompilerOrchestrator,
        MissingOutputArtifactInCompilerOrchestrator,
        DuplicateStageInOrchestrator
    };

    class InternalCompilerException : public std::logic_error {
    private:
        InternalErrorCode code_;

    public:
        template<typename... Args>
        explicit InternalCompilerException(InternalErrorCode code, Args &&... args)
            : std::logic_error("Internal Compiler Error [ICE-" + std::to_string(static_cast<int>(code)) + "]: " +
                               format_error(code, std::forward<Args>(args)...)),
              code_(code) {
        }

        [[nodiscard]] InternalErrorCode get_code() const { return code_; }
    };
}
