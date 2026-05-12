#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentErrorRegistryRunner : public ParserTestBase,
                                          public testing::WithParamInterface<ErrorRegistryEntry<AssignmentVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<AssignmentVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingVariableName", "let = 1",
                {{ValuascriptErrorCode::InvalidIdentifier, 1, 5, 1, 6}},
                IsAssignment({}, {{"<error>"}}, IsNumber("1")));

            reg("InvalidCharacter1", "let a! = 1",
                {{ValuascriptErrorCode::InvalidCharacter, 1, 6, 1, 7}},
                IsAssignment({}, {{"a"}}, IsNumber("1")));

            reg("InvalidCharacter2", "let a ! = 1",
                {{ValuascriptErrorCode::InvalidCharacter, 1, 7, 1, 8}},
                IsAssignment({}, {{"a"}}, IsNumber("1")));

            reg("InvalidVariableNameStart", "let 123 = 1",
                {{ValuascriptErrorCode::InvalidIdentifier, 1, 5, 1, 8}},
                IsAssignment({}, {{"<error>"}}, IsNumber("1")));

            reg("IncompleteAssignmentMissingEquals", "let a 1",
                {{ValuascriptErrorCode::IncompleteAssignment, 1, 7, 1, 8}},
                IsAssignment({}, {{"a"}}, IsNumber("1")));

            reg("IncompleteMultipleAssignmentMissingEquals", "let a, b 1",
                {{ValuascriptErrorCode::IncompleteAssignment, 1, 10, 1, 11}},
                IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1")));

            reg("MissingValueAfterEquals", "let a = ",
                {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 8, 1, 9}},
                IsAssignment({}, {{"a"}}, IsNull()));

            reg("MissingValueAfterEqualsWithTypeAnnotation", "let a: int =",
                {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 13, 1, 14}},
                IsAssignment({}, {{"a", IsType("int")}}, IsNull()));

            reg("IncompleteAssignmentAtEOF", "let a",
                {{ValuascriptErrorCode::IncompleteAssignment, 1, 5, 1, 6}},
                IsAssignment({}, {{"a"}}, IsNull()));

            reg("MultiAssignmentTrailingComma", "let a, = 1",
                {{ValuascriptErrorCode::InvalidIdentifier, 1, 8, 1, 9}},
                IsAssignment({}, {{"a"}, {"<error>"}}, IsNumber("1")));

            reg("MultiAssignmentMissingComma", "let a b = 1",
                {{ValuascriptErrorCode::ExpectedCommaInMultiAssignment, 1, 7, 1, 8}},
                IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1")));

            reg("MultiAssignmentDoubleComma", "let a,, b = 1",
                {{ValuascriptErrorCode::InvalidIdentifier, 1, 7, 1, 8}},
                IsAssignment({}, {{"a"}, {"<error>"}, {"b"}}, IsNumber("1")));

            reg("IllegalModifierOnSingleElementOfMultiAssignment", "let a, @export b = 1",
                {
                    {ValuascriptErrorCode::ModifiersAttachedToMultiAssignmentSingleElements, 1, 8, 1, 15}
                },
                IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1")));

            reg("MissingTypeAfterColon", "let a: = 1",
                {{ValuascriptErrorCode::MissingTypeAnnotation, 1, 8, 1, 9}},
                IsAssignment({}, {{"a", IsNullType()}}, IsNumber("1")));

            reg("BrokenNestedTypeAnnotation", "let a: vector<int = 1",
                {{ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs, 1, 19, 1, 20}},
                IsAssignment({}, {{"a", IsType("vector", {IsType("int")})}}, IsNumber("1")));

            reg("ReservedKeywordAsTarget", "let func = 1",
                {{ValuascriptErrorCode::ReservedKeywordAsIdentifier, 1, 5, 1, 9}},
                IsAssignment({}, {{"func"}}, IsNumber("1")));

            reg("ReservedKeywordInMultiAssignment", "let a, if = 1",
                {{ValuascriptErrorCode::ReservedKeywordAsIdentifier, 1, 8, 1, 10}},
                IsAssignment({}, {{"a"}, {"if"}}, IsNumber("1")));

            reg("MissingModifierNameAfterAt", "@ let a = 1",
                {{ValuascriptErrorCode::ExpectedModifierName, 1, 3, 1, 6}},
                IsAssignment({{"<error>"}}, {{"a"}}, IsNumber("1")));

            reg("MultipleBrokenModifiers", "@mod1 @123 let a = 1",
                {{ValuascriptErrorCode::ExpectedModifierName, 1, 8, 1, 11}},
                IsAssignment({{"mod1"}, {"<error>"}}, {{"a"}}, IsNumber("1")));

            reg("MissingTypeAfterColonInMultiAssignment", "let a: integer, b: = 1",
                {{ValuascriptErrorCode::MissingTypeAnnotation, 1, 20, 1, 21}},
                IsAssignment({}, {{"a", IsType("integer")}, {"b", IsNullType()}}, IsNumber("1")));

            reg("MissingValueAfterEqualsMultiAssignment", "let x, y = ",
                {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 11, 1, 12}},
                IsAssignment({}, {{"x"}, {"y"}}, IsNull()));

            return true;
        }();
    }

    TEST_P(AssignmentErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectAssignmentErrors(code, errors, verifier);
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
