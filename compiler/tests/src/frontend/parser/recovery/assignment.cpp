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
                .errors = {PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "<error>"}}, IsNumber("1"))
            });

            reg({
                .name = "InvalidCharacter1",
                .code = "let a! = 1",
                .errors = {PErr{.code = LexerErrorCode::InvalidCharacter, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}}, IsNumber("1"))
            });

            reg({
                .name = "InvalidCharacter2",
                .code = "let a ! = 1",
                .errors = {PErr{.code = LexerErrorCode::InvalidCharacter, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}}, IsNumber("1"))
            });

            reg({
                .name = "InvalidVariableNameStart",
                .code = "let 123 = 1",
                .errors = {PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 8}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "<error>"}}, IsNumber("1"))
            });

            reg({
                .name = "IncompleteAssignmentMissingEquals",
                .code = "let a 1",
                .errors = {PErr{.code = E::IncompleteAssignment, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}}, IsNumber("1"))
            });

            reg({
                .name = "IncompleteMultipleAssignmentMissingEquals",
                .code = "let a, b 1",
                .errors = {PErr{.code = E::IncompleteAssignment, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}, AssignmentTargetSpec{.name = "b"}}, IsNumber("1"))
            });

            reg({
                .name = "MissingValueAfterEquals",
                .code = "let a = ",
                .errors = {PErr{.code = E::MissingValueAfterEquals, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}}, IsNull())
            });

            reg({
                .name = "MissingValueAfterEqualsWithTypeAnnotation",
                .code = "let a: int =",
                .errors = {PErr{.code = E::MissingValueAfterEquals, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a", .type_v = IsType("int")}}, IsNull())
            });

            reg({
                .name = "IncompleteAssignmentAtEOF",
                .code = "let a",
                .errors = {PErr{.code = E::IncompleteAssignment, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}}, IsNull())
            });

            reg({
                .name = "MultiAssignmentTrailingComma",
                .code = "let a, = 1",
                .errors = {PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}, AssignmentTargetSpec{.name = "<error>"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiAssignmentMissingComma",
                .code = "let a b = 1",
                .errors = {PErr{.code = E::ExpectedCommaInMultiAssignment, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}, AssignmentTargetSpec{.name = "b"}}, IsNumber("1"))
            });

            reg({
                .name = "MultiAssignmentDoubleComma",
                .code = "let a,, b = 1",
                .errors = {PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}, AssignmentTargetSpec{.name = "<error>"}, AssignmentTargetSpec{.name = "b"}}, IsNumber("1"))
            });

            reg({
                .name = "MissingTypeAfterColon",
                .code = "let a: = 1",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a", .type_v = IsNullType()}}, IsNumber("1"))
            });

            reg({
                .name = "BrokenNestedTypeAnnotation",
                .code = "let a: vector<int = 1",
                .errors = {PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 17, .line_end = 1, .column_end = 18}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a", .type_v = IsType("vector", IsType("int"))}}, IsNumber("1"))
            });

            reg({
                .name = "ReservedKeywordAsTarget",
                .code = "let func = 1",
                .errors = {PErr{.code = E::ReservedKeywordAsIdentifier, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 9}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "func"}}, IsNumber("1"))
            });

            reg({
                .name = "ReservedKeywordInMultiAssignment",
                .code = "let a, if = 1",
                .errors = {PErr{.code = E::ReservedKeywordAsIdentifier, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 10}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a"}, AssignmentTargetSpec{.name = "if"}}, IsNumber("1"))
            });

            reg({
                .name = "MissingTypeAfterColonInMultiAssignment",
                .code = "let a: integer, b: = 1",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 20, .line_end = 1, .column_end = 21}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "a", .type_v = IsType("integer")}, AssignmentTargetSpec{.name = "b", .type_v = IsNullType()}}, IsNumber("1"))
            });

            reg({
                .name = "MissingValueAfterEqualsMultiAssignment",
                .code = "let x, y = ",
                .errors = {PErr{.code = E::MissingValueAfterEquals, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}},
                .verifier = IsAssignment({AssignmentTargetSpec{.name = "x"}, AssignmentTargetSpec{.name = "y"}}, IsNull())
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
        TestNameGenerator{}
    );
}
