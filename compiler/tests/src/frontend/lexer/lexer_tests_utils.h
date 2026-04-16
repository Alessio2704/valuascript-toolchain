#pragma once
#include "frontend/lexer/lexer_stage.h"
#include "../../../../../shared/src/token/token.h"
#include <string>
#include <vector>
#include "utils/parametrised_test_name_helper.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    inline std::vector<Token> tokenize_code(const std::string &source_code) {
        LexerStage lexer_stage;

        auto context = std::make_shared<CompilerContext>();

        const std::vector<CompilerStageArtifact> history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, source_code}
        };

        auto [code, data] = lexer_stage.run(*context, history);
        return std::any_cast<std::vector<Token> >(data);
    }
}