#include <gtest/gtest.h>
#include <regex>
#include "errors/diagnostic_formatter.h"
#include "errors/valuascript_exception.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class DiagnosticFormatterTest : public ::testing::Test {
    protected:
        static std::string strip_ansi(const std::string &input) {
            static const std::regex ansi_regex("\033\\[[0-9;]*m");
            return std::regex_replace(input, ansi_regex, "");
        }

        static ValuaScriptException create_dummy_error(ValuascriptErrorCode code, size_t line, size_t col_start,
                                                       size_t col_end) {
            SourceSpan span;
            span.file_path = "test.vs";
            span.line_start = line;
            span.line_end = line;
            span.column_start = col_start;
            span.column_end = col_end;

            return {ValuascriptErrorCategory::Syntax, code, span, "Test error message"};
        }
    };

    TEST_F(DiagnosticFormatterTest, AlignsSquigglesCorrectly) {
        std::string source = "let x = 100 + 200";
        // Let's pretend the error is on '100' (starts at col 9, ends at col 12)
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidExpression, 1, 9, 12);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // We expect the margin " 1 | " (5 chars) + 8 spaces of indent + "^~~"
        std::string expected_line = " 1 | let x = 100 + 200";
        std::string expected_squiggle = "             ^~~";

        EXPECT_TRUE(clean_output.find(expected_line) != std::string::npos)
        << "Failed to print the source line correctly.\nOutput:\n" << clean_output;

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Squiggle alignment is off.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, HandlesMissingSourceLineGracefully) {
        std::string source = "let a = 10\nlet b = 20";
        // Error is on line 5, but source only has 2 lines
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 5, 1, 1);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        EXPECT_TRUE(clean_output.find("<source line not available>") != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, HandlesColumnOneCorrectly) {
        std::string source = "} stray brace";
        // Error exactly at column 1 (needs 0 indent spaces)
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        std::string expected_squiggle = "     ^"; // " 1 | " + "^"

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, AdaptsMarginWidthForLargeLineNumbers) {
        // A file with 100 lines (we'll just simulate it by putting the error on line 100)
        std::string source = "let a = 1\n"; // We only need it to not crash, the extract_line will fail gracefully
        for (int i = 2; i <= 100; i++) source += "let x = " + std::to_string(i) + "\n";

        // Error on line 100, column 5
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidExpression, 100, 5, 6);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // The margin for line 100 is " 100 | " (7 characters)
        // Column 5 means 4 spaces of indent.
        // Total prefix before squiggle: 7 margin spaces + 4 indent spaces = 11 spaces
        std::string expected_margin = " 100 | let x = 100";
        std::string expected_squiggle = "           ^";

        EXPECT_TRUE(clean_output.find(expected_margin) != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Squiggle failed to align with a 3-digit line number margin.\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, SurvivesInvertedOrCorruptedSpans) {
        std::string source = "let bad_span = 10";

        // Parser bug: column_start is 10, but column_end is 5
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 10, 5);

        // This should not crash or allocate a billion squiggles
        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // It should safely default to a length of 1 ("^")
        std::string expected_squiggle = "              ^"; // " 1 | " (5 chars) + 9 spaces

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, HandlesMultiLineSpansByPointingToStart) {
        std::string source = "let dict = {\n  a: 1\n";

        // Span starts on line 1, ends on line 3
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 12, 1);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // Because it's multi-line, the formatter should protect itself and just draw a length-1 caret at the start.
        std::string expected_squiggle = "            ^"; // " 1 | " + 11 spaces

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, DrawsSingleCaretForLengthOneSpans) {
        std::string source = "func test()";
        // Pointing exactly at the ')' at column 11
        auto err = create_dummy_error(ValuascriptErrorCode::MissingArrowInFunction, 1, 11, 11);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // It should print exactly "^", not "^~"
        std::string expected_squiggle = "               ^";

        // We make sure there is no trailing tilde
        EXPECT_TRUE(clean_output.find("^~") == std::string::npos) << "Drew too many squiggles for a length-1 span.";
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, SafelyHandlesEmptySourceStrings) {
        std::string source = "";

        // Error at 1:1 (e.g., Expected declaration)
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // Should fall back to the safe "not available" message without crashing
        EXPECT_TRUE(clean_output.find("<source line not available>") != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, MathematicallyProvesSingleCharacterSpan) {
        std::string source = "let a = 1;";
        // Span exactly on the '=' at column 7
        // Math: col_end (7) - col_start (7) = 0. Length must default to 1 ("^").
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 7, 7);

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(err, source));

        // Margin " 1 | " (5 chars). Indent = 7 - 1 = 6 spaces. Total prefix = 11 chars.
        std::string expected_margin = " 1 | let a = 1;";
        std::string expected_squiggle = "           ^";

        EXPECT_TRUE(clean_output.find(expected_margin) != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Failed single character span math.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, MathematicallyProvesMultiCharacterSpan) {
        std::string source = "let invalid_var = 10;";
        // Span covering 'invalid_var' from column 5 to 16
        // Math: col_end (16) - col_start (5) = 11.
        // The squiggle should be exactly 11 characters long: "^~~~~~~~~~~"
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidIdentifier, 1, 5, 16);

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(err, source));

        // Margin " 1 | " (5 chars). Indent = 5 - 1 = 4 spaces.
        std::string expected_margin = " 1 | let invalid_var = 10;";
        std::string expected_squiggle = "         ^~~~~~~~~~~";

        EXPECT_TRUE(clean_output.find(expected_margin) != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Failed multi-character span math. Expected length 11.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, MathematicallyProvesMultiLineFallback) {
        std::string source = "let obj = {\n  key: 1\n";
        // Span covering a block from Line 1, Col 11 to Line 3, Col 1
        // Math: line_start (1) != line_end (3).
        // The formatter MUST abort length calculation and fallback to length 1 ("^").
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 11, 1);

        // Manually override the line_end since the dummy helper assumes single-line
        auto span = err.get_span();
        span.line_end = 3;
        ValuaScriptException multi_line_err(err.get_category(), err.get_code(), span, "Test");

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(multi_line_err, source));

        // Margin " 1 | " (5 chars). Indent = 11 - 1 = 10 spaces.
        std::string expected_squiggle = "               ^";

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Failed multi-line fallback math. It should not draw tildes.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, MathematicallyProvesCorruptedSpanFallback) {
        std::string source = "let a = 10;";
        // Parser bug simulation: token length calculated backwards
        // Math: col_end (2) < col_start (8). col_end - col_start would overflow an unsigned int.
        // The formatter MUST catch this and fallback to length 1 ("^").
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 8, 2);

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(err, source));

        // Margin " 1 | " (5 chars). Indent = 8 - 1 = 7 spaces.
        std::string expected_squiggle = "            ^";

        EXPECT_TRUE(clean_output.find("~") == std::string::npos)
        << "Formatter drew tildes for a negative length span.";
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Failed corrupted span fallback math.\nOutput:\n" << clean_output;
    }
}
