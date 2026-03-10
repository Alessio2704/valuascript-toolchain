#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    ParserStage::ParserStage()
        : CompilerStage(
            "ParserStage",
            CompilerStageArtifactCode::Ast,
            {CompilerStageArtifactCode::TokenStream, CompilerStageArtifactCode::FilePath}
        ) {
    }

    CompilerStageArtifact ParserStage::run(std::shared_ptr<CompilerContext> context, const std::vector<CompilerStageArtifact> &artifacts) {
        const auto tokens = extract_artifact_data<std::vector<Token> >(
            artifacts, CompilerStageArtifactCode::TokenStream);
        const auto file_path = extract_artifact_data<std::string>(artifacts, CompilerStageArtifactCode::FilePath);

        TokenCursor cursor(tokens, file_path);
        Parser parser(std::move(cursor));

        std::shared_ptr<Program> ast = parser.parse_program();

        return {CompilerStageArtifactCode::Ast, ast};
    }
}
