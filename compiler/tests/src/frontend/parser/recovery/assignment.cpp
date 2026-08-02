#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentErrorRegistryRunner : public ParserTestBase,
                                          public testing::WithParamInterface<ErrorRegistryEntry<AssignmentVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<AssignmentVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingVariableName",
                .code = "let = 1",
                .errors = {{E::ExpectedIdentifier, 1, 5, 1, 6}},
                .verifier = IsAssignment({{"<error>"}}, IsNumber("1"))
            });

            reg({
                .name = "InvalidCharacter1",
                .code = "let a! = 1",
                .errors = {{LexerErrorCode::InvalidCharacter, 1, 6, 1, 7}},
                .verifier = IsAssignment({{"a"}}, IsNumber("1"))
            });

            reg({
                .name = "InvalidCharacter2",
                .code = "let a ! = 1",
                .errors = {{LexerErrorCode::InvalidCharacter, 1, 7, 1, 8}},
                .verifier = IsAssignment({{"a"}}, IsNumber("1"))
            });

            reg({
                .name = "InvalidVariableNameStart",
                .code = "let 123 = 1",
                .errors = {{E::ExpectedIdentifier, 1, 5, 1, 8}},
                .verifier = IsAssignment({{"<error>"}}, IsNumber("1"))
            });

            reg({
                .name = "IncompleteAssignmentMissingEquals",
                .code = "let a 1",
                .errors = {{E::IncompleteAssignment, 1, 7, 1, 8}},
                .verifier = IsAssignment({{"a"}}, IsNumber("1"))
            });

            reg({
                .name = "IncompleteMultipleAssignmentMissingEquals",
                .code = "let a, b 1",
                .errors = {{E::IncompleteAssignment, 1, 10, 1, 11}},
                .verifier = IsAssignment({{"a"}, {"b"}}, IsNumber("1"))
            });

            reg({
                .name = "MissingValueAfterEquals",
                .code = "let a = ",
                .errors = {{E::MissingValueAfterEquals, 1, 8, 1, 9}},
                .verifier = IsAssignment({{"a"}}, IsNull())
            });

            reg({
                .name = "MissingValueAfterEqualsWithTypeAnnotation",
                .code = "let a: int =",
                .errors = {{E::MissingValueAfterEquals, 1, 13, 1, 14}},
                .verifier = IsAssignment({{"a", IsType("int")}}, IsNull())
            });

            reg({
                .name = "IncompleteAssignmentAtEOF",
                .code = "let a",
                .errors = {{E::IncompleteAssignment, 1, 5, 1, 6}},
                .verifier = IsAssignment({{"a"}}, IsNull())
            });

            reg({
                .name = "MultiAssignmentTrailingComma",
                .code = "let a, = 1",
                .errors = {{E::ExpectedIdentifier, 1, 8, 1, 9}},
                .verifier = IsAssignment({{"a"}, {"<error>"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiAssignmentMissingComma",
                .code = "let a b = 1",
                .errors = {{E::ExpectedCommaInMultiAssignment, 1, 7, 1, 8}},
                .verifier = IsAssignment({{"a"}, {"b"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiAssignmentDoubleComma",
                .code = "let a,, b = 1",
                .errors = {{E::ExpectedIdentifier, 1, 7, 1, 8}},
                .verifier = IsAssignment({{"a"}, {"<error>"}, {"b"}}, IsNumber("1"))
            });

            reg({
                .name = "MissingTypeAfterColon",
                .code = "let a: = 1",
                .errors = {{E::MissingTypeAnnotation, 1, 8, 1, 9}},
                .verifier = IsAssignment({{"a", IsNullType()}}, IsNumber("1"))
            });

            reg({
                .name = "BrokenNestedTypeAnnotation",
                .code = "let a: vector<int = 1",
                .errors = {{E::UnmatchedBracketAfterGenericArgs, 1, 17, 1, 18}},
                .verifier = IsAssignment({{"a", IsType("vector", IsType("int"))}}, IsNumber("1"))
            });

            reg({
                .name = "ReservedKeywordAsTarget",
                .code = "let func = 1",
                .errors = {{E::ReservedKeywordAsIdentifier, 1, 5, 1, 9}},
                .verifier = IsAssignment({{"func"}}, IsNumber("1"))
            });

            reg({
                .name = "ReservedKeywordInMultiAssignment",
                .code = "let a, if = 1",
                .errors = {{E::ReservedKeywordAsIdentifier, 1, 8, 1, 10}},
                .verifier = IsAssignment({{"a"}, {"if"}}, IsNumber("1"))
            });

            reg({
                .name = "MissingTypeAfterColonInMultiAssignment",
                .code = "let a: integer, b: = 1",
                .errors = {{E::MissingTypeAnnotation, 1, 20, 1, 21}},
                .verifier = IsAssignment({{"a", IsType("integer")}, {"b", IsNullType()}}, IsNumber("1"))
            });

            reg({
                .name = "MissingValueAfterEqualsMultiAssignment",
                .code = "let x, y = ",
                .errors = {{E::MissingValueAfterEquals, 1, 11, 1, 12}},
                .verifier = IsAssignment({{"x"}, {"y"}}, IsNull())
            });

            return true;
        }();
    }

    TEST_P(AssignmentErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectAssignmentErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Assignment,
        AssignmentErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::assignments()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<AssignmentVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
