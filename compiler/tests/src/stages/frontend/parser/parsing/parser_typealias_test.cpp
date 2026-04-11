#include <gtest/gtest.h>
#include "frontend/parser/ast_base_test.h"
#include "errors/valuascript_exception.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    struct TypeAliasHappyParam {
        std::string test_name;
        std::string source_code;
        std::string expected_alias_name;
        size_t expected_modifier_count;
    };

    class TypeAliasHappyPathTest : public AstBaseTest,
                                   public testing::WithParamInterface<TypeAliasHappyParam> {
    };

    TEST_P(TypeAliasHappyPathTest, ParsesSuccessfully) {
        const TypeAliasHappyParam &param = GetParam();

        std::shared_ptr<Program> ast;
        EXPECT_NO_THROW({
            ast = parse_code(param.source_code);
            }) << "Parser threw an exception on valid type alias test: " << param.test_name;

        if (ast) {
            ASSERT_EQ(ast->type_aliases.size(), 1) << "Expected exactly 1 type alias in AST.";
            EXPECT_EQ(ast->execution_steps.size(), 0);
            EXPECT_EQ(ast->struct_definitions.size(), 0);
            EXPECT_EQ(ast->function_definitions.size(), 0);

            auto type_alias = ast->type_aliases[0].get();
            EXPECT_EQ(type_alias->name, param.expected_alias_name)
                << "Parsed alias name does not match expected name.";
            EXPECT_EQ(type_alias->modifiers.size(), param.expected_modifier_count)
                << "Modifier count mismatch.";
            EXPECT_NE(type_alias->target_type, nullptr)
                << "Expected type alias to have a valid target type annotation.";
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        TypeAliasHappyPathTest,
        testing::Values(
            TypeAliasHappyParam{
            "simple_scalar",
            "typealias Result = scalar",
            "Result", 0
            },
            TypeAliasHappyParam{
            "simple_string",
            "typealias Names = string",
            "Names", 0
            },
            TypeAliasHappyParam{
            "template_generic",
            "typealias NumberList = vector<scalar>",
            "NumberList", 0
            },
            TypeAliasHappyParam{
            "template_nested",
            "typealias Matrix = vector<vector<decimal>>",
            "Matrix", 0
            },
            TypeAliasHappyParam{
            "tuple_type",
            "typealias ConfigPair = (string, bool)",
            "ConfigPair", 0
            },
            TypeAliasHappyParam{
            "complex_tuple",
            "typealias ApiResult = (vector<string>, decimal, bool)",
            "ApiResult", 0
            },
            TypeAliasHappyParam{
            "single_modifier",
            "@export typealias Response = string",
            "Response", 1
            },
            TypeAliasHappyParam{
            "multiple_modifiers",
            "@export @deprecated(arg: \"use V2\") typealias OldStruct = map<string, any>",
            "OldStruct", 2
            },
            TypeAliasHappyParam{
            "multiline_formatting",
            "typealias \n Data \n = \n vector<scalar>",
            "Data", 0
            }
        ),
        [](const testing::TestParamInfo<TypeAliasHappyParam>& info) {
        return info.param.test_name;
        }
    );

    struct TypeAliasSadParam {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class TypeAliasSadPathTest : public AstBaseTest,
                                 public testing::WithParamInterface<TypeAliasSadParam> {
    };

    TEST_P(TypeAliasSadPathTest, ThrowsCorrectSyntaxError) {
        const TypeAliasSadParam &param = GetParam();

        try {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        } catch (const ValuaScriptException &e) {
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
