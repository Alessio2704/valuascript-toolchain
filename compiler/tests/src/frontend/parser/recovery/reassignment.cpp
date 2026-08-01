#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ReassignmentErrorRegistryRunner : public ParserTestBase,
                                            public testing::WithParamInterface<ErrorRegistryEntry<ReassignmentVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ReassignmentVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MultiReassignmentNotSupported",
                .code = "x, y = 1",
                .errors = {{E::MultiReassignmentNotSupported, 1, 2, 1, 3}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement1",
                .code = "x + y",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 6}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement2",
                .code = "a[0] + y",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 9}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement3",
                .code = "a[0].a + y",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 11}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement4",
                .code = "a",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 2}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement5",
                .code = "a[0]",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 5}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement6",
                .code = "{} + a",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 7}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement7",
                .code = "-1",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 3}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement8",
                .code = "1 + 2",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 6}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidStandaloneStatement9",
                .code = "true",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 5}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide1",
                .code = "a() = 1",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 4}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide2",
                .code = "(a, b) = 1",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 7}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide3",
                .code = "[1, 2] = 3",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 7}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide4",
                .code = "true = false",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 5}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide5",
                .code = "self = 42",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 5}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide6",
                .code = "self.calc() = 42",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 12}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide7",
                .code = "{} = 42",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 3}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidLeftSide8",
                .code = "a + b = 3",
                .errors = {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 6}},
                .verifier = IsNull()
            });

            reg({
                .name = "ChainedReassignmentNotSupported",
                .code = "a = b = c = 0",
                .errors = {{E::ChainedAssignmentNotSupported, 1, 5, 1, 14}},
                .verifier = IsReassignment(IsIdentifier("a"), IsNumber("0"))
            });

            reg({
                .name = "ModifiersOnReassignment",
                .code = "@modifier a = 1",
                .errors = {{E::ModifiersAttachedToInvalidDeclaration, 1, 1, 1, 10}},
                .verifier = IsReassignment(IsIdentifier("a"), IsNumber("1"))
            });

            reg({
                .name = "ModifiersOnInvalidStandaloneStatement",
                .code = "@modifier 1 + 2",
                .errors = {
                    {E::ModifiersAttachedToInvalidDeclaration, 1, 1, 1, 10},
                    {E::InvalidStandaloneStatement, 1, 11, 1, 16}
                },
                .verifier = IsNull()
            });

            reg({
                .name = "MissingValueAfterEqualsReassignment",
                .code = "a = ",
                .errors = {{E::MissingValueAfterEquals, 1, 3, 1, 4}},
                .verifier = IsReassignment(IsIdentifier("a"), IsNull())
            });

            return true;
        }();
    }

    TEST_P(ReassignmentErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectReassignmentErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Reassignment,
        ReassignmentErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::reassignments()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<ReassignmentVerifier>>& test_info)
        {
        return test_info.param.test_name;
        }
    );
}
