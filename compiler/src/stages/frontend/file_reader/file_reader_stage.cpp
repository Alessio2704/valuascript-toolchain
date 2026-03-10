#include "stages/frontend/file_reader/file_reader_stage.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace valuascript::compiler {
    FileReaderStage::FileReaderStage()
        : CompilerStage(
            "FileReaderStage",
            CompilerStageArtifactCode::SourceCode,
            {CompilerStageArtifactCode::FilePath}
        ) {
    }

    CompilerStageArtifact FileReaderStage::run(std::shared_ptr<CompilerContext> context, const std::vector<CompilerStageArtifact> &artifacts) {
        auto file_path = extract_artifact_data<std::string>(
            artifacts,
            CompilerStageArtifactCode::FilePath
        );

        std::ifstream file_stream(file_path);
        if (!file_stream.is_open()) {
            throw std::runtime_error("FileReaderStage Error: Cannot open file at path '" + file_path + "'");
        }

        std::ostringstream buffer;
        buffer << file_stream.rdbuf();

        return {CompilerStageArtifactCode::SourceCode, buffer.str()};
    }
}
