#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/error_shifter.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    class ExpansionRecoveryDebugger : public ParserTestBase
    {
    public:
        template <typename Verifier = NullVerifier>
        static void DumpRecoveryExpansion(InjectableType type,
                                          const std::string& snippet,
                                          const std::vector<ParserExpectedError>& errors,
                                          const std::string& label,
                                          const Verifier& verifier = NullVerifier{},
                                          const std::vector<std::string_view>& skip_contexts = {},
                                          const std::vector<ContextOverride<Verifier>>& context_overrides = {})
        {
            DumpWriter writer("expansion_sentinel_recovery_debug_" + label + ".txt", "expansion_dumps");
            if (!writer.is_open()) return;

            auto& out = writer.out();

            out << "============================================================\n";
            out << "RECOVERY EXPANSION DUMP FOR: " << label << "\n";
            out << "Snippet: " << snippet << "\n";
            out << "============================================================\n\n";

            size_t scenario_index = 0;
            size_t base_seed = std::hash<std::string>{}(label);

            expand_to_top_level_stream(type, snippet, verifier, label, [&](ProcessingItem&& item)
            {
                if (item.is_skipped) return;

                ProgramSpec item_spec;
                auto prog = BuildRecoveryProgram(
                    std::move(item.code),
                    std::move(item_spec),
                    std::move(item.cumulative_prefix),
                    base_seed + (scenario_index * 2)
                );

                scenario_index++;

                out << "--- VARIATION " << scenario_index << " ---\n";
                out << "PATH:  " << item.path_name << "\n";
                out << "DEPTH: " << item.depth << "\n";
                out << "EXPECTED SHIFTED ERRORS (Sample calculation):\n";

                const auto& active_errors = item.custom_errors.value_or(errors);
                auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, active_errors);
                for (const auto& err : shifted)
                {
                    out << "  - Code: " << err.code
                        << " at [" << err.line_start << ":" << err.column_start << "]\n";
                }

                out << "FULL CODE:\n";
                out << prog.full_code;
                out << "------------------------------------------------------------\n\n";
            }, true, skip_contexts, context_overrides);

            out << "[DEBUG] Recovery expansion dump finished (" << scenario_index << " variations)";
        }
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_F(ExpansionRecoveryDebugger, GenerateRecoveryReport)
    {
        DumpRecoveryExpansion(
            InjectableType::StrongStatement,
            "let x = ",
            {ParserExpectedError(ParserErrorCode::MissingValueAfterEquals, 1, 7)},
            "BrokenAssignment"
        );

        DumpRecoveryExpansion(
            InjectableType::StrongStatement,
            "1 + 1",
            {ParserExpectedError(ParserErrorCode::InvalidStandaloneStatement, 1, 6)},
            "InvalidStandaloneStatement"
        );

        DumpRecoveryExpansion(
            InjectableType::Expression,
            "1 + ",
            {ParserExpectedError(ParserErrorCode::InvalidExpression, 1, 5)},
            "MalformedBinary"
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "map<string, *, int>",
            {ParserExpectedError(ParserErrorCode::MissingTypeAnnotation, 1, 13)},
            "BrokenTypeAnnotation"
        );

        DumpRecoveryExpansion(
            InjectableType::Modifier,
            "@test(a 1, b: 2)",
            {ParserExpectedError(ParserErrorCode::MissingColonAfterArgument, 1, 9)},
            "BrokenModifier"
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "vector<int",
            {ParserExpectedError(ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 10, 1, 11)},
            "GenericMissingClosingBracket",
            NullVerifier{},
            {
                ContextNames::TypeTupleTypeStart,
                ContextNames::TypeTupleTypeMiddle,
                ContextNames::TypeGenericTypeStart,
                ContextNames::TypeGenericTypeMiddle,
                ContextNames::TypeGenericTypeEnd
            },
            {
                ContextOverride{
                    .context_name = ContextNames::TypeMultiAssignmentTarget1,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 18, 1, 19}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeFunctionMultiReturn,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 15, 1, 16}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeTupleTypeStart,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 15, 1, 16}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeTupleTypeMiddle,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 18, 1, 19}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeGenericTypeStart,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 16, 1, 17}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeGenericTypeMiddle,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 16, 1, 17}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeGenericTypeEnd,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedBracketAfterGenericArgs, 1, 12, 1, 13}
                    }
                },
            }
        );

        DumpRecoveryExpansion(
            InjectableType::TypeAnnotation,
            "(int, string",
            std::vector<ParserExpectedError>{
                {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 12, 1, 13}
            },
            "TupleMissingClosingParen",
            NullVerifier{},
            {
                ContextNames::TypeTupleTypeStart,
                ContextNames::TypeTupleTypeMiddle,
                ContextNames::TypeTupleTypeEnd,
                ContextNames::TypeGenericTypeStart,
                ContextNames::TypeGenericTypeMiddle,
                ContextNames::TypeGenericTypeEnd
            },
            {
                ContextOverride{
                    .context_name = ContextNames::TypeFunctionParameter,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::ExpectedRightParenAfterParameters, 1, 13, 1, 14}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeFunctionMultiParameter2,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::ExpectedRightParenAfterParameters, 1, 13, 1, 14}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeTupleTypeEnd,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 13, 1, 14}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeMultiAssignmentTarget1,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 20, 1, 21}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeFunctionMultiReturn,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 17, 1, 18}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeTupleTypeStart,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 17, 1, 18}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeTupleTypeMiddle,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 20, 1, 21}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeGenericTypeStart,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 18, 1, 19}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeGenericTypeMiddle,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 18, 1, 19}
                    }
                },
                ContextOverride{
                    .context_name = ContextNames::TypeGenericTypeEnd,
                    .errors = std::vector<ParserExpectedError>{
                        {ParserErrorCode::UnmatchedParenthesisInTuple, 1, 14, 1, 15}
                    }
                },
            }
        );
    }
#endif
}
