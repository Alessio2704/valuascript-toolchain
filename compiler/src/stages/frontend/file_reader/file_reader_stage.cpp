#include "stages/frontend/file_reader/file_reader_stage.h"
#include <fstream>
#include <sstream>
#include "errors/error_messages.h"

namespace valuascript::compiler {
    FileReaderStage::FileReaderStage()
        : CompilerStage(
            "FileReaderStage",
            CompilerStageArtifactCode::SourceCode,
            {CompilerStageArtifactCode::FilePath}
        ) {
    }

    CompilerStageArtifact FileReaderStage::run(const std::shared_ptr<CompilerContext> &context,
                                               const std::vector<CompilerStageArtifact> &artifacts) {
        auto file_path = extract_artifact_data<std::string>(
            artifacts,
            CompilerStageArtifactCode::FilePath
        );

        std::ifstream file_stream(file_path);
        if (!file_stream.is_open()) {
            ValuaScriptException ex(
                ErrorCategory::Import,
                ErrorCode::ImportFileNotFound,
                {0, 0, 0, 0, file_path},
                format_error_message(ErrorCode::ImportFileNotFound, file_path)
            );
            context->handle_error(ex);
        }

        std::ostringstream buffer;
        buffer << file_stream.rdbuf();

        context->update_source_registry(file_path, buffer.str());

        return {CompilerStageArtifactCode::SourceCode, buffer.str()};
    }
}
