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
                .errors = {PErr{.code = E::MultiReassignmentNotSupported, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide1",
                .code = "a() = 1",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 4}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide2",
                .code = "(a, b) = 1",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 7}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide3",
                .code = "[1, 2] = 3",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 7}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide4",
                .code = "true = false",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide5",
                .code = "self = 42",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 5}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide6",
                .code = "self.calc() = 42",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 12}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide7",
                .code = "{} = 42",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 3}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "InvalidLeftSide8",
                .code = "a + b = 3",
                .errors = {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 6}},
                .verifier = IsNull(),
                .excluded_pools = { PoolKind::InvalidDeclarationInExpression }
            });

            reg({
                .name = "ChainedReassignmentNotSupported",
                .code = "a = b = c = 0",
                .errors = {PErr{.code = E::ChainedAssignmentNotSupported, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 14}},
                .verifier = IsReassignment(IsIdentifier("a"), IsNumber("0"))
            });

            reg({
                .name = "MissingValueAfterEqualsReassignment",
                .code = "a = ",
                .errors = {PErr{.code = E::MissingValueAfterEquals, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 4}},
                .verifier = IsReassignment(IsIdentifier("a"), IsNull()),
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "MissingPropertyNameInReassignment",
                .code = "self. = 1",
                .errors = {PErr{.code = E::ExpectedPropertyName, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}},
                .verifier = IsReassignment(IsDot(IsSelf(), "<error>"), IsNumber("1"))
            });

            reg({
                .name = "MissingClosingBracketInTensorAccessReassignment",
                .code = "arr[0 = 1",
                .errors = {PErr{.code = E::UnmatchedBracketAfterTensorIndex, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}},
                .verifier = IsReassignment(IsBracket(IsIdentifier("arr"), IsNumber("0")), IsNumber("1"))
            });

            return true;
        }();
    }

    TEST_P(ReassignmentErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& p = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + p.test_name);

        ExpectReassignmentErrors(p.code, p.errors, p.verifier, p.skip_contexts, p.context_overrides, p.excluded_sentinels, p.accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Reassignment,
        ReassignmentErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::reassignments()),
        TestNameGenerator{}
    );
}
