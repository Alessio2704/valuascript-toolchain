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
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidExpression, 1, 9, 12);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // " 1 | " = 5 chars margin + 8 indent spaces = 13 prefix spaces
        std::string expected_line = " 1 | let x = 100 + 200";
        std::string expected_squiggle = "             ^~~";

        EXPECT_TRUE(clean_output.find(expected_line) != std::string::npos)
        << "Failed to print the source line correctly.\nOutput:\n" << clean_output;

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Squiggle alignment is off.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, HandlesMissingSourceLineGracefully) {
        std::string source = "let a = 10\nlet b = 20";
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 5, 1, 1);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        EXPECT_TRUE(clean_output.find("<source line not available>") != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, HandlesColumnOneCorrectly) {
        std::string source = "} stray brace";
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // " 1 | " = 5 chars margin + 0 indent spaces = 5 prefix spaces
        std::string expected_squiggle = "     ^";

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Output:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, AdaptsMarginWidthForLargeLineNumbers) {
        std::string source = "let a = 1\n";
        for (int i = 2; i <= 100; i++) source += "let x = " + std::to_string(i) + "\n";

        auto err = create_dummy_error(ValuascriptErrorCode::InvalidExpression, 100, 5, 6);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // Margin " 100 | " (7 characters) + 4 indent spaces = 11 prefix spaces
        std::string expected_margin = " 100 | let x = 100";
        std::string expected_squiggle = "           ^";

        EXPECT_TRUE(clean_output.find(expected_margin) != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Squiggle failed to align with a 3-digit line number margin.\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, SurvivesInvertedOrCorruptedSpans) {
        std::string source = "let bad_span = 10";

        // Parser bug simulation: column_end < column_start
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 10, 5);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // Margin " 1 | " (5 chars) + 9 indent spaces = 14 prefix spaces
        std::string expected_squiggle = "              ^";

        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Output:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, DrawsSingleCaretForLengthOneSpans) {
        std::string source = "func test()";
        auto err = create_dummy_error(ValuascriptErrorCode::MissingArrowInFunction, 1, 11, 11);

        std::string raw_output = DiagnosticFormatter::format_error(err, source);
        std::string clean_output = strip_ansi(raw_output);

        // Margin " 1 | " (5 chars) + 10 indent spaces = 15 prefix spaces
        std::string expected_squiggle = "               ^";

        EXPECT_TRUE(clean_output.find("^~") == std::string::npos) << "Drew too many squiggles for a length-1 span.";
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos);
    }

    TEST_F(DiagnosticFormatterTest, MathematicallyProvesSingleCharacterSpan) {
        std::string source = "let a = 1;";
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
        auto err = create_dummy_error(ValuascriptErrorCode::InvalidIdentifier, 1, 5, 16);

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(err, source));

        // Margin " 1 | " (5 chars). Indent = 5 - 1 = 4 spaces. Prefix = 9 chars.
        std::string expected_margin = " 1 | let invalid_var = 10;";
        std::string expected_squiggle = "         ^~~~~~~~~~~";

        EXPECT_TRUE(clean_output.find(expected_margin) != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
        << "Failed multi-character span math. Expected length 11.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, FormatsMultiLineSpansCorrectly) {
        std::string source = "let obj = {\n  key: 1\n}";

        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 11, 2);
        auto span = err.get_span();
        span.line_end = 3;
        ValuaScriptException multi_line_err(err.get_category(), err.get_code(), span, "Test");

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(multi_line_err, source));

        // pipe_margin = 3 spaces.
        // Pointer: "   " + "|___" + "__________" + "^" (10 dynamic underscores + 3 hardcoded = 13 underscores)
        std::string expected_pointer = "   |_____________^";

        EXPECT_TRUE(clean_output.find(" 1 | / let obj = {") != std::string::npos);
        EXPECT_TRUE(clean_output.find(" 2 | |   key: 1") != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_pointer) != std::string::npos)
        << "Failed multi-line pointer formatting.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, TruncatesExtremelyLongMultiLineSpans) {
        std::string source = "func test() {\n";
        for (int i = 2; i <= 10; i++) source += "  let x" + std::to_string(i) + " = " + std::to_string(i) + "\n";
        source += "}";

        auto err = create_dummy_error(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1);
        auto span = err.get_span();
        span.line_end = 11;
        ValuaScriptException multi_line_err(err.get_category(), err.get_code(), span, "Test");

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(multi_line_err, source));

        // padding = 2. pipe_margin = 4 spaces.
        // col = 1 -> spaces = 0.
        // Pointer: "    " + "|___" + "" + "^" -> "    |___^"
        std::string expected_pointer = "    |___^";

        EXPECT_TRUE(clean_output.find(" 1 | / func test() {") != std::string::npos);
        EXPECT_TRUE(clean_output.find(" 2 | |   let x2 = 2") != std::string::npos);
        EXPECT_TRUE(clean_output.find("    | ...") != std::string::npos);
        EXPECT_TRUE(clean_output.find("10 | |   let x10 = 10") != std::string::npos);
        EXPECT_TRUE(clean_output.find("11 | | }") != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_pointer) != std::string::npos)
        << "Failed long multi-line truncation formatting.\nOutput:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, FormatsSingleLineNestedFunction) {
        std::string source = "func test() -> void {\n    func test_inside() -> void {}\n}";

        // Error on line 2, from col 5 to col 34 (length of "func test_inside() -> void {}")
        auto err = create_dummy_error(ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere, 2, 5, 34);

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(err, source));

        // Padding for single digit '2' is 1. Margin " 2 | " is 5 chars.
        // Indent is 4 spaces (column 5). Total prefix before squiggle: 9 chars.
        std::string expected_margin = " 2 |     func test_inside() -> void {}";
        std::string expected_squiggle = "         ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

        EXPECT_TRUE(clean_output.find(expected_margin) != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_squiggle) != std::string::npos)
            << "Output did not match expected single-line format:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, FormatsShortMultiLineNestedFunction) {
        std::string source =
                "func test() -> void {\n"
                "    func test_inside() -> void {\n"
                "        let a = 1\n"
                "    }\n"
                "}";

        auto err = create_dummy_error(ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere, 2, 5, 2);
        auto span = err.get_span();
        span.line_end = 4;
        span.column_end = 6;
        ValuaScriptException multi_line_err(err.get_category(), err.get_code(), span, "Test error");

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(multi_line_err, source));

        // padding = 1. pipe_margin = 3 spaces. spaces = 4 (col 5 - 1).
        // Pointer offset: "   " + "|___" + "____" + "^"
        // 3 hardcoded underscores + 4 dynamic = 7 underscores!
        std::string expected_pointer = "   |_______^";

        EXPECT_TRUE(clean_output.find(" 2 | /     func test_inside() -> void {") != std::string::npos);
        EXPECT_TRUE(clean_output.find(" 3 | |         let a = 1") != std::string::npos);
        EXPECT_TRUE(clean_output.find(" 4 | |     }") != std::string::npos);
        EXPECT_TRUE(clean_output.find("...") == std::string::npos) << "Formatter truncated a block <= 6 lines!";
        EXPECT_TRUE(clean_output.find(expected_pointer) != std::string::npos)
            << "Output did not match expected short multi-line format:\n" << clean_output;
    }

    TEST_F(DiagnosticFormatterTest, FormatsLongMultiLineNestedFunctionWithTruncation) {
        std::string source =
                "func test() -> void {\n"
                "    func test_inside() -> void {\n"
                "        let a = 1\n"
                "        let b = 1\n"
                "        let c = 1\n"
                "        let d = 1\n"
                "        let e = 1\n"
                "        let f = 1\n"
                "        let g = 1\n"
                "        let h = 1\n"
                "        let i = 1\n"
                "        let l = 1\n"
                "        let m = 1\n"
                "        let n = 1\n"
                "    }\n"
                "}";

        auto err = create_dummy_error(ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere, 2, 5, 2);
        auto span = err.get_span();
        span.line_end = 15;
        span.column_end = 6;
        ValuaScriptException multi_line_err(err.get_category(), err.get_code(), span, "Test error");

        std::string clean_output = strip_ansi(DiagnosticFormatter::format_error(multi_line_err, source));

        // padding = 2. pipe_margin = 4 spaces. spaces = 4 (col 5 - 1).
        // Pointer offset: "    " + "|___" + "____" + "^"
        // 3 hardcoded underscores + 4 dynamic = 7 underscores!
        std::string expected_pointer = "    |_______^";

        EXPECT_TRUE(clean_output.find("  2 | /     func test_inside() -> void {") != std::string::npos);
        EXPECT_TRUE(clean_output.find("  3 | |         let a = 1") != std::string::npos);
        EXPECT_TRUE(clean_output.find("  4 | |         let b = 1") != std::string::npos);
        EXPECT_TRUE(clean_output.find("    | ...") != std::string::npos) << "Formatter failed to truncate!";
        EXPECT_TRUE(clean_output.find(" 14 | |         let n = 1") != std::string::npos);
        EXPECT_TRUE(clean_output.find(" 15 | |     }") != std::string::npos);
        EXPECT_TRUE(clean_output.find(expected_pointer) != std::string::npos)
            << "Output did not match expected truncated multi-line format:\n" << clean_output;
    }
}
