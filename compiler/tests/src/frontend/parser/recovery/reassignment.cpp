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
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<ReassignmentVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MultiReassignmentNotSupported",
                "x, y = 1",
                {{E::MultiReassignmentNotSupported, 1, 2, 1, 3}},
                IsNull()
            );

            reg("InvalidStandaloneStatement1",
                "x + y",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 6}},
                IsNull()
            );

            reg("InvalidStandaloneStatement2",
                "a[0] + y",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 9}},
                IsNull()
            );

            reg("InvalidStandaloneStatement3",
                "a[0].a + y",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 11}},
                IsNull()
            );

            reg("InvalidStandaloneStatement4",
                "a",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 2}},
                IsNull()
            );

            reg("InvalidStandaloneStatement5",
                "a[0]",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 5}},
                IsNull()
            );

            reg("InvalidStandaloneStatement6",
                "{} + a",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 7}},
                IsNull()
            );

            reg("InvalidStandaloneStatement7",
                "-1",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 3}},
                IsNull()
            );

            reg("InvalidStandaloneStatement8",
                "1 + 2",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 6}},
                IsNull()
            );

            reg("InvalidStandaloneStatement9",
                "true",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 5}},
                IsNull()
            );

            reg("InvalidLeftSide1",
                "a() = 1",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 4}},
                IsNull()
            );

            reg("InvalidLeftSide2",
                "(a, b) = 1",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 7}},
                IsNull()
            );

            reg("InvalidLeftSide3",
                "[1, 2] = 3",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 7}},
                IsNull()
            );

            reg("InvalidLeftSide4",
                "true = false",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 5}},
                IsNull()
            );

            reg("InvalidLeftSide5",
                "self = 42",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 5}},
                IsNull()
            );

            reg("InvalidLeftSide6",
                "self.calc() = 42",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 12}},
                IsNull()
            );

            reg("InvalidLeftSide7",
                "{} = 42",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 3}},
                IsNull()
            );

            reg("InvalidLeftSide8",
                "a + b = 3",
                {{E::InvalidLeftSideExpressionInReassignment, 1, 1, 1, 6}},
                IsNull()
            );

            reg("ChainedReassignmentNotSupported",
                "a = b = c = 0",
                {{E::ChainedAssignmentNotSupported, 1, 5, 1, 14}},
                IsReassignment(IsIdentifier("a"), IsNumber("0"))
            );

            reg("ModifiersOnReassignment",
                "@modifier a = 1",
                {{E::ModifiersAttachedToInvalidDeclaration, 1, 1, 1, 10}},
                IsReassignment(IsIdentifier("a"), IsNumber("1"))
            );

            reg("ModifiersOnInvalidStandaloneStatement",
                "@modifier 1 + 2",
                {
                    {E::ModifiersAttachedToInvalidDeclaration, 1, 1, 1, 10},
                    {E::InvalidStandaloneStatement, 1, 11, 1, 16}
                },
                IsNull()
            );

            reg("MissingValueAfterEqualsReassignment",
                "a = ",
                {{E::MissingValueAfterEquals, 1, 3, 1, 4}},
                IsReassignment(IsIdentifier("a"), IsNull())
            );

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
