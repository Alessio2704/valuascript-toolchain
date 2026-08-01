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
    };

    struct ExpectedToken
    {
        TokenType type;
        std::optional<std::string> lexeme = std::nullopt;
        std::optional<size_t> line = std::nullopt;
        std::optional<size_t> column = std::nullopt;

        ExpectedToken(TokenType t) : type(t) {}
        ExpectedToken(TokenType t, std::string l) : type(t), lexeme(std::move(l)) {}
        ExpectedToken(TokenType t, std::string l, size_t li, size_t c)
            : type(t), lexeme(std::move(l)), line(li), column(c) {}
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
                {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
                {CompilerStageArtifactCode::SourceCode, source_code}
            };

            LexerStage lexer_stage;
            auto result_artifact = lexer_stage.run(*context, history);
            return extract_artifact_data<std::vector<Token>>({result_artifact}, CompilerStageArtifactCode::TokenStream);
        }

        static void ExpectTokens(const std::string& source_code,
                                 const std::vector<ExpectedToken>& expected_tokens)
        {
            std::vector<Token> actual_tokens;
            ASSERT_NO_THROW({
                actual_tokens = tokenize_code(source_code);
            }) << "Lexer threw an unexpected exception for source: " << source_code;

            ASSERT_EQ(actual_tokens.size(), expected_tokens.size())
                << "Token count mismatch for source: \"" << source_code << "\"";

            for (size_t i = 0; i < expected_tokens.size(); ++i)
            {
                EXPECT_EQ(actual_tokens[i].type, expected_tokens[i].type)
                    << "Token type mismatch at index " << i << " for source: \"" << source_code << "\"";

                if (expected_tokens[i].lexeme.has_value())
                {
                    EXPECT_EQ(actual_tokens[i].lexeme, expected_tokens[i].lexeme.value())
                        << "Lexeme mismatch at index " << i << " for source: \"" << source_code << "\"";
                }

                if (expected_tokens[i].line.has_value())
                {
                    EXPECT_EQ(actual_tokens[i].line, expected_tokens[i].line.value())
                        << "Line mismatch at index " << i << " for source: \"" << source_code << "\"";
                }

                if (expected_tokens[i].column.has_value())
                {
                    EXPECT_EQ(actual_tokens[i].column, expected_tokens[i].column.value())
                        << "Column mismatch at index " << i << " for source: \"" << source_code << "\"";
                }
            }
        }

        static void ExpectTokens(const std::string& source_code,
                                 const std::vector<TokenType>& expected_types)
        {
            std::vector<ExpectedToken> expected;
            expected.reserve(expected_types.size());
            for (auto t : expected_types)
            {
                expected.emplace_back(t);
            }
            ExpectTokens(source_code, expected);
        }

        static void ExpectLexerError(const std::string& source_code, LexerErrorCode expected_error)
        {
            try
            {
                tokenize_code(source_code);
                FAIL() << "Expected lexer error " << static_cast<int>(expected_error)
                       << " but lexing succeeded for source: \"" << source_code << "\"";
            }
            catch (const ValuaScriptException& e)
            {
                EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Lexical)
                    << "Error category mismatch for source: \"" << source_code << "\"";
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
            }
        }

        static void ExpectLexerRecovery(const std::string& source_code,
                                        const std::vector<LexerExpectedError>& expected_errors,
                                        const std::vector<ExpectedToken>& expected_tokens)
        {
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
            }

            ASSERT_EQ(actual_tokens.size(), expected_tokens.size())
                << "Token count mismatch in recovery for source: \"" << source_code << "\"";

            for (size_t i = 0; i < expected_tokens.size(); ++i)
            {
                EXPECT_EQ(actual_tokens[i].type, expected_tokens[i].type)
                    << "Token type mismatch at index " << i << " for source: \"" << source_code << "\"";

                if (expected_tokens[i].lexeme.has_value())
                {
                    EXPECT_EQ(actual_tokens[i].lexeme, expected_tokens[i].lexeme.value())
                        << "Lexeme mismatch at index " << i << " for source: \"" << source_code << "\"";
                }

                if (expected_tokens[i].line.has_value())
                {
                    EXPECT_EQ(actual_tokens[i].line, expected_tokens[i].line.value())
                        << "Line mismatch at index " << i << " for source: \"" << source_code << "\"";
                }

                if (expected_tokens[i].column.has_value())
                {
                    EXPECT_EQ(actual_tokens[i].column, expected_tokens[i].column.value())
                        << "Column mismatch at index " << i << " for source: \"" << source_code << "\"";
                }
            }
        }
    };
}
