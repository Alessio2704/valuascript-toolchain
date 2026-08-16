#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <gtest/gtest.h>
#include "core/compiler_context.h"
#include "test_structures.h"

namespace valuascript::compiler
{
    class Program;
}

namespace valuascript::compiler::test
{
    class ParserRunner
    {
    public:
        static std::string format_source_with_lines(const std::string& code);
        static std::string format_affected_source_snippet(const std::string& code, const std::vector<size_t>& error_lines, int context_lines = 8);
        static std::shared_ptr<Program> run_parser(const std::string& code, CompilerContext& context);

        static void ExpectValidParse(const std::string& code, const ProgramSpec& spec);
        static void ExpectParseErrors(const std::string& code,
                                       const std::vector<ParserExpectedError>& expected_errors,
                                       const std::optional<ProgramSpec>& spec = std::nullopt);
        static void ExpectParseErrorsWithRecovery(const std::string& code,
                                                   const std::vector<ParserExpectedError>& expected_errors,
                                                   ProgramSpec broken_part_spec);
    };
}
