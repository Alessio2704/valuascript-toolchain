#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/parser_stage.h"
#include "compiler_context/compiler_context.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

struct ExpectedParserError {
    ValuascriptErrorCode code;
    size_t line;
    size_t column;
};

struct ParserErrorsSynchronizationTestCase {
    std::string test_name;
    std::string source_code;
    std::vector<ExpectedParserError> expected_errors;
};

class ParserErrorsSynchronizationBase : public testing::TestWithParam<ParserErrorsSynchronizationTestCase> {
protected:
    static void run_parser_and_check_errors(const ParserErrorsSynchronizationTestCase &param) {
        auto context = std::make_shared<CompilerContext>();
        context->settings.fail_fast = false;

        std::vector<CompilerStageArtifact> initial_artifacts = {
            {CompilerStageArtifactCode::SourceCode, param.source_code},
            {CompilerStageArtifactCode::FilePath, std::string("test_script.vs")}
        };

        LexerStage lexer;
        ParserStage parser;

        auto lexer_artifacts = initial_artifacts;
        lexer_artifacts.push_back(lexer.run(*context, initial_artifacts));

        ASSERT_NO_THROW({
            parser.run(*context, lexer_artifacts);
            }) << "Parser threw an exception even though fail_fast was set to false.";

        const auto &actual_errors = context->diagnostics.get_errors();

        ASSERT_EQ(actual_errors.size(), param.expected_errors.size())
            << "Mismatch in the number of collected errors.\n"
            << "Expected " << param.expected_errors.size() << ", but got " << actual_errors.size();

        for (size_t i = 0; i < actual_errors.size(); ++i) {
            const auto &actual = actual_errors[i];
            const auto &expected = param.expected_errors[i];
            std::cout << actual.what();

            EXPECT_EQ(actual.get_code(), expected.code)
                << "Error [" << i << "] Code mismatch.\nExpected Code: " << static_cast<int>(expected.code)
                << "\nActual Code: " << static_cast<int>(actual.get_code())
                << "\nActual Message: " << actual.what();

            EXPECT_EQ(actual.get_span().line_start, expected.line)
                << "Error [" << i << "] Line mismatch for error: " << actual.what();

            EXPECT_EQ(actual.get_span().column_end, expected.column)
                << "Error [" << i << "] Column mismatch for error: " << actual.what();
        }
    }
};