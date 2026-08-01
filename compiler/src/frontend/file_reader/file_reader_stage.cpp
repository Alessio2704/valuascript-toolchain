#include "file_reader_stage.h"
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include "file_reader_error_code.h"
#include "core/error_formatter.h"

namespace valuascript::compiler
{
    FileReaderStage::FileReaderStage()
        : CompilerStage(
            "FileReaderStage",
            CompilerStageArtifactCode::SourceCode,
            {CompilerStageArtifactCode::FilePath}
        )
    {
    }

    CompilerStageArtifact FileReaderStage::run(CompilerContext& context,
                                               const std::vector<CompilerStageArtifact>& artifacts)
    {
        const auto& raw_file_path = extract_artifact_data<std::string>(
            artifacts,
            CompilerStageArtifactCode::FilePath
        );

        std::string canonical_path = std::filesystem::weakly_canonical(raw_file_path).string();

        std::ifstream file_stream(canonical_path, std::ios::in | std::ios::binary);
        if (!file_stream.is_open())
        {
            ValuaScriptException ex(
                ValuascriptErrorCategory::File,
                FileReaderErrorCode::FileNotFound,
                {0, 0, 0, 0, canonical_path},
                format_error(FileReaderErrorCode::FileNotFound, canonical_path)
            );
            context.handle_error(ex);
        }

        std::ostringstream buffer;
        buffer << file_stream.rdbuf();
        std::string source_content = buffer.str();

        std::erase(source_content, '\r');

        context.source_manager.update_source(canonical_path, source_content);

        return {CompilerStageArtifactCode::SourceCode, source_content};
    }
}
