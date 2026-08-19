#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include "frontend/lexer/lexer_stage.h"
#include "frontend/lexer/lexer_error_code.h"
#include "token/token.h"
#include "core/compiler_context.h"
#include "core/valuascript_exception.h"
#include "utils/parametrised_test_name_helper.h"

namespace valuascript::compiler::test
{
    struct LexerExpectedError
    {
        LexerErrorCode code;
        size_t line;
        size_t column;
        size_t start_offset = 0;
        size_t length = 0;
    };

    struct ExpectedToken
    {
        TokenType type = TokenType::Error;
        std::string lexeme = {};
        size_t line = 0;
        size_t column = 0;
        size_t start_offset = 0;
        size_t length = 0;
    };

    class LexerTestBase : public testing::Test
    {
    protected:
        static std::vector<Token> tokenize_code(const std::string& source_code,
                                                bool fail_fast = true,
                                                std::shared_ptr<CompilerContext> context = nullptr)
        {
            if (!context)
            {
                thread_local static auto default_test_context = std::make_shared<CompilerContext>();
                context = default_test_context;
            }
            context->settings.fail_fast = fail_fast;

            const std::vector<CompilerStageArtifact> history = {
                {.code = CompilerStageArtifactCode::FilePath, .data = std::string("test.vs")},
                {.code = CompilerStageArtifactCode::SourceCode, .data = source_code}
            };

            LexerStage lexer_stage;
            auto result_artifact = lexer_stage.run(*context, history);
            return extract_artifact_data<std::vector<Token>>({result_artifact}, CompilerStageArtifactCode::TokenStream);
        }

        static void ExpectTokens(const std::string& source_code,
                                 const std::vector<ExpectedToken>& expected_tokens)
        {
            for (size_t i = 0; i < expected_tokens.size(); ++i)
            {
                EXPECT_NE(expected_tokens[i].type, TokenType::EndOfFile)
                    << "Do not explicitly pass EndOfFile to ExpectTokens; it is checked automatically at the end.";
            }

            std::vector<Token> actual_tokens;
            ASSERT_NO_THROW({
                actual_tokens = tokenize_code(source_code);
            }) << "Lexer threw an unexpected exception for source: " << source_code;

            ASSERT_FALSE(actual_tokens.empty())
                << "Lexer returned empty token stream for source: \"" << source_code << "\"";

            EXPECT_EQ(actual_tokens.back().type, TokenType::EndOfFile)
                << "Last token must always be EndOfFile for source: \"" << source_code << "\"";

            ASSERT_EQ(actual_tokens.size(), expected_tokens.size() + 1)
                << "Token count mismatch (excluding trailing EOF) for source: \"" << source_code << "\"";

            for (size_t i = 0; i < expected_tokens.size(); ++i)
            {
                EXPECT_EQ(actual_tokens[i].type, expected_tokens[i].type)
                    << "Token type mismatch at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].lexeme, expected_tokens[i].lexeme)
                    << "Lexeme mismatch at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].line, expected_tokens[i].line)
                    << "Line mismatch at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].column, expected_tokens[i].column)
                    << "Column mismatch at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].start_offset, expected_tokens[i].start_offset)
                    << "Start offset mismatch at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].length, expected_tokens[i].length)
                    << "Length mismatch at index " << i << " for source: \"" << source_code << "\"";
            }
        }

        static void ExpectTokens(const std::string& source_code)
        {
            ExpectTokens(source_code, std::vector<ExpectedToken>{});
        }

        static void ExpectTokens(const std::string& source_code,
                                 const std::vector<TokenType>& expected_types)
        {
            auto actual_tokens = tokenize_code(source_code);
            ASSERT_FALSE(actual_tokens.empty());
            EXPECT_EQ(actual_tokens.back().type, TokenType::EndOfFile);
            ASSERT_EQ(actual_tokens.size(), expected_types.size() + 1);
            for (size_t i = 0; i < expected_types.size(); ++i)
            {
                EXPECT_EQ(actual_tokens[i].type, expected_types[i]);
            }
        }

        static void ExpectLexerError(const std::string& source_code, LexerErrorCode expected_error)
        {
            try
            {
                tokenize_code(source_code);
                FAIL() << "Expected lexer to throw ValuaScriptException for source: \"" << source_code << "\"";
            }
            catch (const ValuaScriptException& e)
            {
                EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Lexical)
                    << "Expected Lexical error category, got: " << static_cast<int>(e.get_category())
                    << " for source: \"" << source_code << "\"";

                EXPECT_TRUE(e.is_error(expected_error))
                    << "Error code mismatch for source: \"" << source_code << "\". Actual error: " << e.what();
            }
            catch (...)
            {
                FAIL() << "Lexer threw an unknown exception for source: \"" << source_code << "\"";
            }
        }

        static void ExpectLexerErrors(const std::string& source_code,
                                      const std::vector<LexerExpectedError>& expected_errors)
        {
            auto context = std::make_shared<CompilerContext>();
            std::vector<Token> tokens;
            ASSERT_NO_THROW({
                tokens = tokenize_code(source_code, /*fail_fast=*/false, context);
            }) << "Lexer threw an exception in non-fail-fast mode for source: \"" << source_code << "\"";

            const auto& actual_errors = context->diagnostics.get_errors();
            ASSERT_EQ(actual_errors.size(), expected_errors.size())
                << "Error count mismatch.\nExpected " << expected_errors.size()
                << ", got " << actual_errors.size() << " for source:\n" << source_code;

            for (size_t i = 0; i < actual_errors.size(); ++i)
            {
                const auto& actual = actual_errors[i];
                const auto& expected = expected_errors[i];

                EXPECT_EQ(actual.get_category(), ValuascriptErrorCategory::Lexical)
                    << "Error category mismatch at error index " << i;

                EXPECT_TRUE(actual.is_error(expected.code))
                    << "Error code mismatch at index " << i
                    << "\nExpected: " << static_cast<int>(expected.code)
                    << "\nActual: " << actual.get_error_number()
                    << "\nMessage: " << actual.what();

                EXPECT_EQ(actual.get_span().line_start, expected.line)
                    << "Line mismatch at error index " << i << " (" << actual.what() << ")";

                EXPECT_EQ(actual.get_span().column_start, expected.column)
                    << "Column mismatch at error index " << i << " (" << actual.what() << ")";

                if (expected.start_offset > 0 || expected.length > 0)
                {
                    EXPECT_EQ(actual.get_span().start_offset, expected.start_offset)
                        << "Start offset mismatch at error index " << i;
                    EXPECT_EQ(actual.get_span().length, expected.length)
                        << "Length mismatch at error index " << i;
                }
            }
        }

        static void ExpectLexerRecovery(const std::string& source_code,
                                        const std::vector<LexerExpectedError>& expected_errors,
                                        const std::vector<ExpectedToken>& expected_tokens)
        {
            for (size_t i = 0; i < expected_tokens.size(); ++i)
            {
                EXPECT_NE(expected_tokens[i].type, TokenType::EndOfFile)
                    << "Do not explicitly pass EndOfFile to ExpectLexerRecovery; it is checked automatically at the end.";
            }

            auto context = std::make_shared<CompilerContext>();
            std::vector<Token> actual_tokens;
            ASSERT_NO_THROW({
                actual_tokens = tokenize_code(source_code, /*fail_fast=*/false, context);
            }) << "Lexer threw an exception in non-fail-fast recovery mode for source: \"" << source_code << "\"";

            const auto& actual_errors = context->diagnostics.get_errors();
            ASSERT_EQ(actual_errors.size(), expected_errors.size())
                << "Error count mismatch.\nExpected " << expected_errors.size()
                << ", got " << actual_errors.size() << " for source:\n" << source_code;

            for (size_t i = 0; i < actual_errors.size(); ++i)
            {
                const auto& actual = actual_errors[i];
                const auto& expected = expected_errors[i];

                EXPECT_EQ(actual.get_category(), ValuascriptErrorCategory::Lexical)
                    << "Error category mismatch at error index " << i;

                EXPECT_TRUE(actual.is_error(expected.code))
                    << "Error code mismatch at index " << i
                    << "\nExpected: " << static_cast<int>(expected.code)
                    << "\nActual: " << actual.get_error_number()
                    << "\nMessage: " << actual.what();

                EXPECT_EQ(actual.get_span().line_start, expected.line)
                    << "Line mismatch at error index " << i << " (" << actual.what() << ")";

                EXPECT_EQ(actual.get_span().column_start, expected.column)
                    << "Column mismatch at error index " << i << " (" << actual.what() << ")";

                if (expected.start_offset > 0 || expected.length > 0)
                {
                    EXPECT_EQ(actual.get_span().start_offset, expected.start_offset)
                        << "Start offset mismatch at error index " << i;
                    EXPECT_EQ(actual.get_span().length, expected.length)
                        << "Length mismatch at error index " << i;
                }
            }

            ASSERT_FALSE(actual_tokens.empty())
                << "Lexer returned empty token stream in recovery for source: \"" << source_code << "\"";

            EXPECT_EQ(actual_tokens.back().type, TokenType::EndOfFile)
                << "Last token in recovery must always be EndOfFile for source: \"" << source_code << "\"";

            ASSERT_EQ(actual_tokens.size(), expected_tokens.size() + 1)
                << "Token count mismatch in recovery (excluding trailing EOF) for source: \"" << source_code << "\"";

            for (size_t i = 0; i < expected_tokens.size(); ++i)
            {
                EXPECT_EQ(actual_tokens[i].type, expected_tokens[i].type)
                    << "Token type mismatch in recovery at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].lexeme, expected_tokens[i].lexeme)
                    << "Lexeme mismatch in recovery at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].line, expected_tokens[i].line)
                    << "Line mismatch in recovery at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].column, expected_tokens[i].column)
                    << "Column mismatch in recovery at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].start_offset, expected_tokens[i].start_offset)
                    << "Start offset mismatch in recovery at index " << i << " for source: \"" << source_code << "\"";
                EXPECT_EQ(actual_tokens[i].length, expected_tokens[i].length)
                    << "Length mismatch in recovery at index " << i << " for source: \"" << source_code << "\"";
            }
        }
    };
}
