#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct TypeAliasSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
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
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        TypeAliasSadPathTest,
        testing::Values(
            TypeAliasSadParam{.test_name = "missing_assignment", .source_code = "typealias MyType scalar", .expected_error = E::ExpectedAssignAfterTypeAliasName
            },
            TypeAliasSadParam{.test_name = "missing_assignment_multiline", .source_code = "typealias MyType \n scalar", .expected_error = E::ExpectedAssignAfterTypeAliasName
            }
        ),
        [](const testing::TestParamInfo<TypeAliasSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
