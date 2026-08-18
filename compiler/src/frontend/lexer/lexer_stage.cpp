#include "lexer_stage.h"
#include "lexer.h"

namespace valuascript::compiler {

    LexerStage::LexerStage()
        : CompilerStage(
            "LexerStage",
            CompilerStageArtifactCode::TokenStream,
            {CompilerStageArtifactCode::SourceCode, CompilerStageArtifactCode::FilePath}
        ) {
    }

    CompilerStageArtifact LexerStage::run(CompilerContext &context,
                                          const std::vector<CompilerStageArtifact> &artifacts) {
        const auto& source = extract_artifact_data<std::string>(artifacts, CompilerStageArtifactCode::SourceCode);
        const auto& file_path = extract_artifact_data<std::string>(artifacts, CompilerStageArtifactCode::FilePath);

        Lexer lexer(source, file_path, context);
        std::vector<Token> tokens = lexer.tokenize();

        return {.code = CompilerStageArtifactCode::TokenStream, .data = tokens};
    }
}
