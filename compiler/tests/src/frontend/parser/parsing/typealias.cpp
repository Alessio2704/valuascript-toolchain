#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct TypeAliasSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class TypeAliasSadPathTest : public AstBaseTest,
                                 public testing::WithParamInterface<TypeAliasSadParam>
    {
    };

    TEST_P(TypeAliasSadPathTest, ThrowsCorrectSyntaxError)
    {
        const TypeAliasSadParam& param = GetParam();

        try
        {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        }
        catch (const ValuaScriptException& e)
        {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
                << "Category mismatch on test: " << param.test_name;
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        TypeAliasSadPathTest,
        testing::Values(
            TypeAliasSadParam{
            "missing_name",
            "typealias = scalar",
            ValuascriptErrorCode::ExpectedTypeAliasName
            },
            TypeAliasSadParam{
            "missing_assignment",
            "typealias MyType scalar",
            ValuascriptErrorCode::ExpectedAssignAfterTypeAliasName
            },
            TypeAliasSadParam{
            "missing_type",
            "typealias MyType =",
            ValuascriptErrorCode::MissingTypeAnnotation
            },
            TypeAliasSadParam{
            "reserved_keyword_as_name",
            "typealias struct = scalar",
            ValuascriptErrorCode::ReservedKeywordAsIdentifier
            },
            TypeAliasSadParam{
            "invalid_target_type_number",
            "typealias MyType = 123",
            ValuascriptErrorCode::MissingTypeAnnotation
            },
            TypeAliasSadParam{
            "missing_assignment_multiline",
            "typealias MyType \n scalar",
            ValuascriptErrorCode::ExpectedAssignAfterTypeAliasName
            },
            TypeAliasSadParam{
            "modifiers_but_missing_name",
            "@export typealias = string",
            ValuascriptErrorCode::ExpectedTypeAliasName
            },
            TypeAliasSadParam{
            "broken_generic_target",
            "typealias List = vector<",
            ValuascriptErrorCode::EmptyGenericTypeAnnotation
            },
            TypeAliasSadParam{
            "unclosed_tuple_target",
            "typealias Pair = (string, scalar",
            ValuascriptErrorCode::UnmatchedParenthesisInTuple
            }
        ),
        [](const testing::TestParamInfo<TypeAliasSadParam>& info) {
        return info.param.test_name;
        }
    );
}
